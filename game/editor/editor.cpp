#include "editor.hpp"

#include "ui_editor.h"

#include <QMouseEvent>

#include "logic/builtin_tileset.hpp"

#include <ffengine/gl/resource.hpp>
#include <ffengine/gl/fbo.hpp>
#include <ffengine/render/scenegraph.hpp>
#include <ffengine/render/camera.hpp>
#include <ffengine/render/renderpass.hpp>
#include <ffengine/render/fullscreenquad.hpp>

#include "openglscene.hpp"

#include "logic/physicsconfig.hpp"
#include "logic/level.hpp"
#include "logic/wall_object.hpp"
#include "logic/rock_object.hpp"

#include "render/visual_object.hpp"

#include "materials.hpp"


class GridNode: public ffe::scenegraph::Node
{
public:
    static constexpr unsigned gridlines =
            /* one for each column and one for each row */
            /* one for the remaining right/bottom borders (top/left are covered by first row/col) */
            /* four for the currently highlighted cell */
            level_width + 1 + level_height + 1;

    static constexpr unsigned vert_offset = (level_width + 1)*2;

public:
    GridNode(ffe::Material &material):
        m_material(material),
        m_ibo_alloc(material.ibo().allocate(gridlines*2)),
        m_highlight_ibo_alloc(material.ibo().allocate(4*2)),
        m_vbo_alloc(material.vbo().allocate(gridlines*2)),
        m_render_highlight(false)
    {
        /* first, all the horizontal grid points */
        std::uint16_t offset = 0;
        std::uint16_t *ibo_out = m_ibo_alloc.get();
        auto positions = ffe::VBOSlice<Vector2f>(m_vbo_alloc, 0);
        for (CoordInt x = 0; x <= level_width; ++x) {
            positions[offset] = Vector2f(x, 0.f);
            positions[offset+1] = Vector2f(x, level_height);

            *ibo_out++ = offset;
            *ibo_out++ = offset+1;
            offset += 2;
        }

        for (CoordInt y = 0; y <= level_height; ++y) {
            positions[offset] = Vector2f(0.f, y);
            positions[offset+1] = Vector2f(level_width, y);

            *ibo_out++ = offset;
            *ibo_out++ = offset+1;
            offset += 2;
        }

        m_ibo_alloc.mark_dirty();
        m_vbo_alloc.mark_dirty();

        m_material.sync_buffers();
    }

private:
    ffe::Material &m_material;

    ffe::IBOAllocation m_ibo_alloc;
    ffe::IBOAllocation m_highlight_ibo_alloc;
    ffe::VBOAllocation m_vbo_alloc;

    CoordPair m_highlight;

    bool m_render_highlight;

public:
    void prepare(ffe::RenderContext&) override
    {

    }

    void render(ffe::RenderContext &context) override
    {
        context.render_all(AABB(), GL_LINES, m_material, m_ibo_alloc, m_vbo_alloc,
                           [](ffe::MaterialPass &pass){
            glUniform1f(pass.shader().uniform_location("intensity"), 0.6f);
        });
        if (m_render_highlight) {
            context.render_all(AABB(), GL_LINES,
                               m_material,
                               m_highlight_ibo_alloc,
                               m_vbo_alloc,
                               [](ffe::MaterialPass &pass){
                glUniform1f(pass.shader().uniform_location("intensity"), 1.f);
            });
        }
    }

    void sync() override
    {
        if (m_highlight.x >= 0 && m_highlight.y >= 0 &&
                m_highlight.x < level_width && m_highlight.y < level_height)
        {
            const uint16_t x = static_cast<uint16_t>(m_highlight.x);
            const uint16_t y = static_cast<uint16_t>(m_highlight.y);
            uint16_t *ibo_out = m_highlight_ibo_alloc.get();
            *ibo_out++ = x*2;
            *ibo_out++ = x*2 + 1;
            *ibo_out++ = (x+1)*2;
            *ibo_out++ = (x+1)*2 + 1;
            *ibo_out++ = y*2 + vert_offset;
            *ibo_out++ = y*2 + vert_offset + 1;
            *ibo_out++ = (y+1)*2 + vert_offset;
            *ibo_out++ = (y+1)*2 + vert_offset + 1;
            m_highlight_ibo_alloc.mark_dirty();
            m_render_highlight = true;
        }
        m_material.sync_buffers();
    }

    void set_highlight(CoordPair pos)
    {
        m_highlight = pos;
    }

};

static float gauss(const float x, const float sigma_sq)
{
    const float sigma_sq2 = sigma_sq * 2.f;
    return 1.f/std::sqrt(M_PIf32*sigma_sq2) * std::exp(-x*x/sigma_sq2);
}

static void compute_blur_vector_to_texture_1d(const unsigned size)
{
    std::vector<float> buffer(size);
    float sum = 0.f;
    for (unsigned int x = 0; x < size; ++x) {
        const float xf = (2.f * static_cast<float>(x) / size - 1.f) * 1.f;
        buffer[x] = gauss(xf, 0.2f);
        sum += buffer[x];
    }
    const float scale = 1.f / sum;
    for (float &dest: buffer) {
        dest *= scale;
    }

    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, size, GL_RED, GL_FLOAT,
                    static_cast<const void*>(buffer.data()));
}


struct EditorScene
{
    ffe::GLResourceManager m_resources;
    ffe::WindowRenderTarget m_window;
    ffe::SceneGraph m_scenegraph;
    ffe::OrthogonalCamera m_camera;
    ffe::Scene m_scene;
    ffe::RenderGraph m_rendergraph;

    ffe::FBO &m_solid_buffer;
    ffe::Texture2D &m_solid_colour;
    ffe::FBO &m_ghost_h_buffer;
    ffe::Texture2D &m_ghost_h_colour;
    ffe::Texture1D &m_blur_vector;

    ffe::RenderPass &m_main_pass;
    ffe::RenderPass &m_ghost_h_pass;
    ffe::RenderPass &m_ghost_v_pass;

    ffe::VBO m_grid_vbo;
    ffe::IBO m_grid_ibo;
    ffe::Material &m_grid_material;

    GridNode &m_grid;

    ffe::FullScreenQuadNode &m_ghost_shader_node;
    ffe::MaterialPass &m_ghost_h_shader;
    ffe::MaterialPass &m_ghost_v_shader;

    EditorScene(Level &level, const QSize window_size):
        m_scene(m_scenegraph, m_camera),
        m_rendergraph(m_scene),
        m_solid_buffer(m_resources.emplace<ffe::FBO>(
                           "fbo/solid",
                           window_size.width(),
                           window_size.height())),
        m_solid_colour(m_resources.emplace<ffe::Texture2D>(
                           "fbo/solid/colour",
                           GL_RGBA32F, window_size.width(), window_size.height(),
                           GL_RGBA, GL_FLOAT)),
        m_ghost_h_buffer(m_resources.emplace<ffe::FBO>(
                           "fbo/ghost_h",
                           window_size.width(),
                           window_size.height())),
        m_ghost_h_colour(m_resources.emplace<ffe::Texture2D>(
                             "fbo/ghost_h/colour",
                             GL_RGBA32F, window_size.width(), window_size.height(),
                             GL_RGBA, GL_FLOAT)),
        m_blur_vector(m_resources.emplace<ffe::Texture1D>(
                          "effects/blur/vector",
                          GL_R32F, 65,
                          GL_RED, GL_FLOAT)),
        m_main_pass(m_rendergraph.new_node<ffe::RenderPass>(m_solid_buffer)),
        m_ghost_h_pass(m_rendergraph.new_node<ffe::RenderPass>(m_ghost_h_buffer)),
        m_ghost_v_pass(m_rendergraph.new_node<ffe::RenderPass>(m_window)),
        m_grid_vbo(ffe::VBOFormat({ffe::VBOAttribute(3)})),
        m_grid_material(m_resources.emplace<ffe::Material>("mat/editor/grid", m_grid_vbo, m_grid_ibo)),
        m_grid(m_scenegraph.root().emplace<GridNode>(m_grid_material)),
        m_ghost_shader_node(m_scenegraph.root().emplace<ffe::FullScreenQuadNode>()),
        m_ghost_h_shader(m_ghost_shader_node.make_pass_material(m_ghost_h_pass)),
        m_ghost_v_shader(m_ghost_shader_node.make_pass_material(m_ghost_v_pass))
    {
        m_camera.controller().set_distance(level_width);
        m_camera.controller().set_rot(Vector2f(0, 0));
        m_camera.controller().set_pos(Vector3f(level_width/2, level_height/2, 0));

        m_main_pass.set_clear_mask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        m_main_pass.set_clear_colour(Vector4f(0.4f, 0.3f, 0.2f, 1.f));

        m_solid_colour.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        m_ghost_h_colour.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        m_blur_vector.bind();
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        compute_blur_vector_to_texture_1d(65);

        m_ghost_h_pass.dependencies().push_back(&m_main_pass);

        m_ghost_v_pass.set_blit_colour_src(&m_solid_buffer);
        m_ghost_v_pass.dependencies().push_back(&m_ghost_h_pass);

        {
            spp::EvaluationContext ctx(m_resources.shader_library());
            ffe::MaterialPass &pass = m_grid_material.make_pass_material(m_main_pass);
            bool success = true;

            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/editor/grid.vert"),
                        ctx,
                        GL_VERTEX_SHADER);
            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/editor/grid.frag"),
                        ctx,
                        GL_FRAGMENT_SHADER);

            m_grid_material.declare_attribute("position", 0);

            success = success && m_grid_material.link();

            if (!success) {
                throw std::runtime_error("failed to compile or link grid material");
            }

            pass.set_depth_test(false);
            pass.set_order(10000);
        }

        const std::array<std::tuple<ffe::MaterialPass&, const char*, ffe::Texture2D*>, 2> ghost_passes{{{m_ghost_h_shader, "H", &m_solid_colour}, {m_ghost_v_shader, "V", &m_ghost_h_colour}}};

        for (auto pass_info: ghost_passes) {
            spp::EvaluationContext ctx(m_resources.shader_library());
            ctx.define1ll("BLUR_SIZE", 17);
            ctx.define1f("BLUR_AMPLIFY", 4.f);
            ctx.define(std::string("DIRECTION_") + std::get<1>(pass_info), "1");

            ffe::MaterialPass &pass = std::get<0>(pass_info);
            bool success = true;

            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/effects/fullscreenquad.vert"),
                        ctx,
                        GL_VERTEX_SHADER);
            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/effects/ghostplane.frag"),
                        ctx,
                        GL_FRAGMENT_SHADER);

            success = success && pass.link();

            if (!success) {
                throw std::runtime_error("failed to compile or link ghost shader material");
            }

            pass.attach_texture("scene_colour", std::get<2>(pass_info));
            pass.attach_texture("gauss", &m_blur_vector);

            pass.shader().bind();
            glUniform1f(pass.shader().uniform_location("scale"), 1.f/window_size.width());
        }

        create_fallback_material(m_resources, m_main_pass);
        create_simple_material(m_resources, m_main_pass, "safe_wall",
                               ":/tileset/wall_pillar.png");
        create_simple_material(m_resources, m_main_pass, "rock",
                               ":/tileset/rock.png");

        m_rendergraph.resort();

        m_solid_buffer.attach(GL_COLOR_ATTACHMENT0, &m_solid_colour);
        m_solid_buffer.make_depth_buffer();

        m_ghost_h_buffer.attach(GL_COLOR_ATTACHMENT0, &m_ghost_h_colour);
        m_ghost_h_buffer.make_depth_buffer();

        m_scenegraph.root().emplace<LevelView>(m_resources, level, true);
    }

    void update_size(const QSize &new_size)
    {
        if (m_window.width() != new_size.width() ||
                m_window.height() != new_size.height() )
        {
            // resize window
            m_window.set_size(new_size.width(), new_size.height());

            // resize FBOs
            const std::array<std::tuple<ffe::FBO&, ffe::Texture2D&>, 2> fbos{{{m_solid_buffer, m_solid_colour}, {m_ghost_h_buffer, m_ghost_h_colour}}};
            for (auto fbo_info: fbos) {
                ffe::FBO &fbo = std::get<0>(fbo_info);
                ffe::Texture2D &texture = std::get<1>(fbo_info);
                fbo = ffe::FBO(new_size.width(), new_size.height());
                texture.bind();
                texture.reinit(GL_RGBA32F, new_size.width(), new_size.height(),
                               GL_RGBA, GL_FLOAT);
                fbo.make_depth_buffer();
                fbo.attach(GL_COLOR_ATTACHMENT0, &texture);
            }

            const Vector2f fetch_scale(new_size.width(),
                                       new_size.height());

            const Vector2f fetch_offset(1.f/new_size.width(),
                                        1.f/new_size.height());

            m_ghost_h_shader.shader().bind();
            glUniform2fv(m_ghost_h_shader.shader().uniform_location("fetch_scale"), 1,
                         fetch_scale.as_array);
            glUniform2fv(m_ghost_h_shader.shader().uniform_location("fetch_offset"), 1,
                         fetch_offset.as_array);
            m_ghost_v_shader.shader().bind();
            glUniform2fv(m_ghost_v_shader.shader().uniform_location("fetch_scale"), 1,
                         fetch_scale.as_array);
            glUniform2fv(m_ghost_h_shader.shader().uniform_location("fetch_offset"), 1,
                         fetch_offset.as_array);

        }
    }
};


TilesetModel::TilesetModel(const Tileset &tileset):
    m_tiles(tileset.tiles())
{

}

TilesetModel::~TilesetModel()
{

}

int TilesetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_tiles.size();
}

QVariant TilesetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const TilesetTile &info = *m_tiles[index.row()];

    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(std::string(info.display_name()));
    case ROLE_TILE_UUID:
        return info.id();
    default:
        return QVariant();
    }
}


class CameraDrag: public WorldToolDrag
{
public:
    CameraDrag(ffe::OrthogonalCamera &camera,
               const Vector2f &viewport_size,
               const Vector2f &start_scene_pos);

private:
    ffe::CameraController &m_controller;
    const Vector2f m_start_scene_pos;

private:
    void drag_camera(const Vector2f &viewport_pos,
                     const Vector2f &world_pos);

};

CameraDrag::CameraDrag(ffe::OrthogonalCamera &camera,
                       const Vector2f &viewport_size,
                       const Vector2f &start_scene_pos):
    WorldToolDrag(camera, viewport_size,
                  std::bind(&CameraDrag::drag_camera,
                            this,
                            std::placeholders::_1,
                            std::placeholders::_2),
                  nullptr),
    m_controller(camera.controller()),
    m_start_scene_pos(start_scene_pos)
{

}

void CameraDrag::drag_camera(const Vector2f&, const Vector2f &world_pos)
{
    const Vector2f diff = m_start_scene_pos - world_pos;
    const Vector2f new_pos = Vector2f(m_controller.pos()) + diff;
    m_controller.set_pos(Vector3f(new_pos, 0.f));
}


Editor::Editor(Application &app, QWidget *parent):
    ApplicationMode(app, parent),
    m_ui(new Ui::Editor()),
    m_tileset(make_builtin_tileset()),
    m_model(std::make_unique<TilesetModel>(*m_tileset)),
    m_mouse_mode(MM_NONE),
    m_distort_t(0)
{
    m_ui->setupUi(this);
    m_ui->listView->setModel(m_model.get());
    setMouseTracking(true);
}

Editor::~Editor()
{

}

Vector2f Editor::mouse_to_scene(const QPoint p)
{
    if (!m_scene) {
        return Vector2f(NAN, NAN);
    }

    const Matrix4f inv_proj = std::get<1>(m_scene->m_camera.render_projection(
                m_scene->m_window.width(),
                m_scene->m_window.height()
                ));
    const Matrix4f inv_view = m_scene->m_camera.calc_inv_view();
    Vector2f mouse(p.x(), p.y());
    mouse *= window()->devicePixelRatioF();
    const Vector3f ndc(2.f * mouse[eX] / m_scene->m_window.width() - 1.f,
                       -(2.f * mouse[eY] / m_scene->m_window.height() - 1.f),
                       0.f);
    const Vector4f scene_pos = inv_view * inv_proj * Vector4f(ndc, 1.f);
    return Vector2f(scene_pos[eX], scene_pos[eY]);
}

std::pair<bool, CoordPair> Editor::scene_to_world(const Vector2f p)
{
    const CoordPair v(std::floor(p[eX]), std::floor(p[eY]));
    /* we do the validity check on the floating point values to avoid issues
     * with over/underflow when flooring to int */
    const bool valid = (p[eX] >= 0 &&
                        p[eY] >= 0 &&
                        p[eX] < level_width &&
                        p[eY] < level_height);
    return std::make_pair(valid, v);
}

void Editor::advance(ffe::TimeInterval dt)
{
    m_distort_t += dt;
}

void Editor::after_gl_sync()
{

}

void Editor::before_gl_sync()
{
    const QSize size = window()->size() * window()->devicePixelRatioF();
    if (!m_scene) {
        m_scene = std::make_unique<EditorScene>(*m_level, size);
    }
    m_scene->update_size(size);
    m_scene->m_window.set_fbo_id(m_gl_scene->defaultFramebufferObject());

    const float mix = static_cast<float>(m_ui->ghost_mix->value()) / m_ui->ghost_mix->maximum();

    m_scene->m_camera.sync();

    const Vector2f scene_pos = mouse_to_scene(m_local_mouse_pos);
    m_scene->m_grid.set_highlight(
                CoordPair(std::floor(scene_pos[eX]),
                          std::floor(scene_pos[eY])));

    Vector2f blur_scale_vector(1.f/subdivision_count, 1.f/subdivision_count);
    /* this is essentially the projection matrix used forward */
    blur_scale_vector *= std::min(size.width(), size.height());
    blur_scale_vector[eX] /= size.width();
    blur_scale_vector[eY] /= size.height();

    /* and this is the view matrix used forward ... */
    blur_scale_vector /= m_scene->m_camera.controller().distance();

    const std::array<std::tuple<ffe::ShaderProgram&, unsigned int>, 2> shaders{{{m_scene->m_ghost_h_shader.shader(), 0}, {m_scene->m_ghost_v_shader.shader(), 1}}};
    for (auto shader_info: shaders) {
        ffe::ShaderProgram &shader = std::get<0>(shader_info);
        const float scale = blur_scale_vector[std::get<1>(shader_info)];
        shader.bind();
        glUniform1f(shader.uniform_location("mix_factor"), mix);
        glUniform1f(shader.uniform_location("distort_t"), m_distort_t);
        glUniform1f(shader.uniform_location("scale"), scale);
    }

    m_scene->m_scenegraph.sync();
    m_gl_scene->setup_scene(&m_scene->m_rendergraph);

    ffe::raise_last_gl_error();
}


void Editor::activate(QWidget &parent)
{
    ApplicationMode::activate(parent);

    m_level = std::make_unique<Level>(level_width, level_height);
    m_level->physics().reset_unblocked_cells();

    m_advance_conn = connect(
                m_gl_scene,
                &OpenGLScene::advance,
                this,
                &Editor::advance,
                Qt::DirectConnection
                );
    m_before_gl_sync_conn = connect(
                m_gl_scene,
                &OpenGLScene::before_gl_sync,
                this,
                &Editor::before_gl_sync,
                Qt::DirectConnection
                );
    m_after_gl_sync_conn = connect(
                m_gl_scene,
                &OpenGLScene::after_gl_sync,
                this,
                &Editor::after_gl_sync,
                Qt::DirectConnection
                );

    m_gl_scene->update();
    setFocus();
}

void Editor::deactivate()
{
    disconnect(m_after_gl_sync_conn);
    disconnect(m_advance_conn);
    disconnect(m_before_gl_sync_conn);

    m_scene = nullptr;
    m_level = nullptr;

    ApplicationMode::deactivate();
}


void Editor::mouseMoveEvent(QMouseEvent *event)
{
    m_local_mouse_pos = event->pos();
    switch (m_mouse_mode) {
    case MM_TOOL_DRAG:
    {
        m_active_mouse_drag->drag(Vector2f(m_local_mouse_pos.x(),
                                           m_local_mouse_pos.y()) * window()->devicePixelRatioF());
        break;
    }
    default:;
    }
}


void Editor::mousePressEvent(QMouseEvent *event)
{
    if (m_mouse_mode != MM_NONE) {
        return;
    }

    if (event->button() == Qt::MiddleButton) {
        m_drag_button = event->button();
        m_mouse_mode = MM_TOOL_DRAG;
        m_active_mouse_drag = std::make_unique<CameraDrag>(
                    m_scene->m_camera,
                    m_viewport_size,
                    mouse_to_scene(event->pos()));
        m_drag_button = event->button();
    } else if (event->button() ==Qt::LeftButton) {
        bool valid;
        CoordPair p;
        std::tie(valid, p) = scene_to_world(mouse_to_scene(event->pos()));
        if (!valid) {
            return;
        }
        QModelIndex current = m_ui->listView->selectionModel()->currentIndex();
        if (!current.isValid()) {
            return;
        }
        QUuid id = current.data(ROLE_TILE_UUID).toUuid();
        auto tile = m_tileset->make_tile(id, *m_level, TileArgv());
        m_level->place_object(std::move(tile), p.x, p.y, 1.0f);
    }
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
    switch (m_mouse_mode) {
    case MM_TOOL_DRAG:
    {
        if (event->button() == m_drag_button) {
            m_mouse_mode = MM_NONE;
            m_active_mouse_drag->done(Vector2f(event->pos().x(),
                                               event->pos().y()) * window()->devicePixelRatioF());
            m_active_mouse_drag = nullptr;
        }
    }
    default:;
    }
}


void Editor::wheelEvent(QWheelEvent *event)
{
    if (event->orientation() == Qt::Vertical) {
        float dist = m_scene->m_camera.controller().distance();
        dist = clamp(dist - event->delta() / 50.f, 1.f,
                     static_cast<float>(std::max(level_width, level_height)));
        m_scene->m_camera.controller().set_distance(dist);
    }
}

void Editor::resizeEvent(QResizeEvent *event)
{
    ApplicationMode::resizeEvent(event);
    const QSize size = m_gl_scene->size() * window()->devicePixelRatioF();
    m_viewport_size = Vector2f(size.width(), size.height());
}
