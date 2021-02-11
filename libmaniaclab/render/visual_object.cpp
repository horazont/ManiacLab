#include "visual_object.hpp"

#include "logic/wall_object.hpp"
#include "logic/player_object.hpp"
#include "logic/dirt_object.hpp"
#include "logic/rock_object.hpp"

#include "logic/level.hpp"


static io::Logger &logger = io::logging().get_logger("maniaclab.render.visual_object");


GameObjectVisual::GameObjectVisual(ffe::ResourceManager &resources,
                                   GameObject &obj,
                                   const Neighbourhood &neigh):
    m_obj(obj),
    m_material(nullptr),
    m_nframes(1),
    m_nvariants(1),
    m_variant(0),
    m_frame_rate(0.f),
    m_frame(0.f),
    m_scale(1.f),
    m_static(false)
{
    /* FIXME: use more proper way to select material ... */
    /* FIXME: take neighbourhood into account ... */
    const char *mat_name = nullptr;
    if (dynamic_cast<SafeWallObject*>(&obj)) {
        mat_name = "mat/object/safe_wall";
        m_nvariants = 2;
        if (dynamic_cast<SafeWallObject*>(neigh[2]) && dynamic_cast<SafeWallObject*>(neigh[3])) {
            // vertical
            m_variant = 1;
        }
    } else if (dynamic_cast<DirtObject*>(&obj)) {
        mat_name = "mat/object/dirt";
    } else if (dynamic_cast<RockObject*>(&obj)) {
        mat_name = "mat/object/rock";
        m_scale = 1.1f;
    } else if (dynamic_cast<PlayerObject*>(&obj)) {
        mat_name = "mat/object/player";
        m_nframes = 16;
        m_frame_rate = 22.f;
    }

    if (mat_name) {
        m_material = resources.get_safe<ffe::Material>(mat_name);
    } else {
        logger.logf(io::LOG_ERROR, "unhandled game object: %p", &obj);
    }

    if (!m_material) {
        logger.logf(io::LOG_WARNING, "failed to find material for object %p (mat_name = %s)", &obj, mat_name);
        m_material = resources.get_safe<ffe::Material>("mat/object/fallback");
    }

    if (!m_material) {
        logger.logf(io::LOG_ERROR, "failed to find fallback material");
        return;
    }

    m_ibo_alloc = m_material->ibo().allocate(6);
    m_vbo_alloc = m_material->vbo().allocate(4);

    uint16_t *dest = m_ibo_alloc.get();
    *dest++ = 0;
    *dest++ = 2;
    *dest++ = 1;
    *dest++ = 3;

    m_ibo_alloc.mark_dirty();
}

GameObjectVisual::~GameObjectVisual()
{
    logger.logf(io::LOG_DEBUG, "cleaning up object visual for %p",
                &m_obj);
}

void GameObjectVisual::advance(ffe::TimeInterval seconds)
{
    if (m_obj.movement) {
        m_frame = std::fmod(m_frame + seconds * m_frame_rate, static_cast<float>(m_nframes));
    } else {
        m_frame = 0.f;
    }
}

void GameObjectVisual::prepare(ffe::RenderContext&)
{

}

void GameObjectVisual::render(ffe::RenderContext &context)
{
    if (m_ibo_alloc && m_vbo_alloc) {
        context.render_all(AABB(), GL_TRIANGLE_STRIP, *m_material, m_ibo_alloc, m_vbo_alloc);
    } else {
        logger.logf(io::LOG_WARNING, "visual for %p has no buffer allocations", &m_obj);
    }
}

void GameObjectVisual::sync()
{
    if (!m_vbo_alloc) {
        return;
    }

    auto position = ffe::VBOSlice<Vector4f>(m_vbo_alloc, 0);
    auto tc0 = ffe::VBOSlice<Vector2f>(m_vbo_alloc, 1);

    const float x = m_static ? m_obj.x : m_obj.visual_pos[eX];
    const float y = m_static ? m_obj.y : m_obj.visual_pos[eY];

    const float s_step = 1.f/m_nframes;
    const float curr_frame = m_static ? 0.0f : std::floor(m_frame);
    const float s0 = s_step * curr_frame;
    const float s1 = s_step * (curr_frame + 1.f);
    const float t_step = 1.f/m_nvariants;
    const float t0 = t_step * m_variant;
    const float t1 = t_step * (m_variant + 1.f);

    const Vector2f offset(x + 0.5f, y + 0.5f);
    unsigned i2 = 2;
    unsigned i3 = 3;
    Matrix2f mat = rotation2(m_obj.phi);
    if (m_obj.flip) {
        mat = mat * scale2(Vector2f(-1.f, 1.f));
        i2 = 3;
        i3 = 2;
    }
    mat = mat * scale2(Vector2f(m_scale, m_scale));

    position[0] = Vector4f(offset + mat * Vector2f(-0.5f, -0.5f), 0.f, 1.f);
    tc0[0] = Vector2f(s0, t0);
    position[1] = Vector4f(offset + mat * Vector2f(0.5f, -0.5f), 0.f, 1.f);
    tc0[1] = Vector2f(s1, t0);
    position[i2] = Vector4f(offset + mat * Vector2f(-0.5f, 0.5f), 0.f, 1.f);
    tc0[i2] = Vector2f(s0, t1);
    position[i3] = Vector4f(offset + mat * Vector2f(0.5f, 0.5f), 0.f, 1.f);
    tc0[i3] = Vector2f(s1, t1);

    m_vbo_alloc.mark_dirty();
    m_material->sync_buffers();
}


LevelView::LevelView(ffe::ResourceManager &resources, Level &level, bool editor_mode):
    m_resources(resources),
    m_level(level),
    m_editor_mode(editor_mode)
{
    m_level.on_object_spawn().connect(std::bind(&LevelView::object_created, this, std::placeholders::_1, std::placeholders::_2));
    for (CoordInt y = 0; y < m_level.height(); ++y) {
        for (CoordInt x = 0; x < m_level.width(); ++x) {
            LevelCell *cell = m_level.get_cell(x, y);
            assert(cell);
            GameObject *obj = cell->here.get();
            if (!obj) {
                continue;
            }
            object_created(m_level, *obj);
        }
    }
}

static GameObject *safe_get_object(Level &level, CoordInt x, CoordInt y)
{
    if (x < 0 || x >= level.width() || y < 0 || y >= level.height()) {
        return nullptr;
    }
    LevelCell *cell = level.get_cell(x, y);
    if (!cell) {
        return nullptr;
    }
    return cell->here.get();
}

void LevelView::object_created(Level &level, GameObject &obj)
{
    assert(&level == &m_level);
    const GameObjectVisual::Neighbourhood neigh{{
            safe_get_object(level, obj.cell.x-1, obj.cell.y),
            safe_get_object(level, obj.cell.x+1, obj.cell.y),
            safe_get_object(level, obj.cell.x, obj.cell.y-1),
            safe_get_object(level, obj.cell.x, obj.cell.y+1)
                                          }};
    auto &visual = emplace<GameObjectVisual>(m_resources, obj, neigh);
    visual.set_static(m_editor_mode);
    auto view = std::make_unique<ObjectView>(*this, visual);
    obj.set_view(std::move(view));
}

void LevelView::object_deleted(GameObjectVisual &visual)
{
    /* this might need optimisation later on */
    for (auto iter = begin(); iter != end(); ++iter) {
        if (&*iter == &visual) {
            erase(iter);
            return;
        }
    }
}

LevelView::ObjectView::ObjectView(LevelView &view, GameObjectVisual &visual):
    m_view(view),
    m_visual(visual)
{

}

LevelView::ObjectView::~ObjectView()
{
    m_view.object_deleted(m_visual);
}


