#include "ingame.hpp"

#include "ui_ingame.h"

#include <fstream>

#include <ffengine/gl/resource.hpp>

#include <QKeyEvent>

#include "openglscene.hpp"

#include "logic/wall_object.hpp"
#include "logic/fog_object.hpp"
#include "logic/bomb_object.hpp"
#include "logic/fan_object.hpp"
#include "logic/dirt_object.hpp"
#include "logic/player_object.hpp"
#include "logic/rock_object.hpp"
#include "logic/builtin_tileset.hpp"
#include "render/visual_object.hpp"

#include "materials.hpp"

static io::Logger &logger = io::logging().get_logger("maniaclab.ingame");


class PhysicsDebugNode: public ffe::scenegraph::Node
{
public:
    PhysicsDebugNode(ffe::Material &material):
        m_material(material),
        m_ibo_alloc(material.ibo().allocate(12)),
        m_vbo_alloc(material.vbo().allocate(4))
    {
        {
            // front and back
            uint16_t *dest = m_ibo_alloc.get();
            *dest++ = 0;
            *dest++ = 1;
            *dest++ = 2;

            *dest++ = 2;
            *dest++ = 1;
            *dest++ = 3;

            *dest++ = 0;
            *dest++ = 2;
            *dest++ = 1;

            *dest++ = 1;
            *dest++ = 2;
            *dest++ = 3;
        }
        m_ibo_alloc.mark_dirty();

        auto positions = ffe::VBOSlice<Vector3f>(m_vbo_alloc, 0);
        auto tc0 = ffe::VBOSlice<Vector2f>(m_vbo_alloc, 1);

        positions[0] = Vector3f(0.f, 0.f, 0);
        positions[1] = Vector3f(level_width, 0.f, 0);
        positions[2] = Vector3f(0.f, level_height, 0);
        positions[3] = Vector3f(level_width, level_height, 0);

        float width = level_width * subdivision_count;
        float height = level_height * subdivision_count;
        float v_margin = 0 * subdivision_count / height;
        float h_margin = 0 * subdivision_count / width;

        tc0[0] = Vector2f(h_margin, v_margin);
        tc0[1] = Vector2f(1-h_margin, v_margin);
        tc0[2] = Vector2f(h_margin, 1-v_margin);
        tc0[3] = Vector2f(1-v_margin, 1-v_margin);

        m_vbo_alloc.mark_dirty();
        m_material.sync_buffers();
    }

private:
    ffe::Material &m_material;

    ffe::IBOAllocation m_ibo_alloc;
    ffe::VBOAllocation m_vbo_alloc;

public:
    void prepare(ffe::RenderContext&) override
    {

    }

    void render(ffe::RenderContext &context) override
    {
        context.render_all(AABB(), GL_TRIANGLES, m_material, m_ibo_alloc, m_vbo_alloc);
    }

    void sync() override
    {

    }

};


class PhysicsParticlesDebugNode: public ffe::scenegraph::Node
{
public:
    PhysicsParticlesDebugNode(ffe::Material &material, ParticleSystem &particles):
        m_material(material),
        m_particles(particles)
    {

    }

private:
    ffe::Material &m_material;
    ParticleSystem &m_particles;

    ffe::IBOAllocation m_ibo_alloc;
    ffe::VBOAllocation m_vbo_alloc;

public:
    void prepare(ffe::RenderContext&) override
    {

    }

    void render(ffe::RenderContext &context) override
    {
        if (m_ibo_alloc && m_vbo_alloc) {
            context.render_all(AABB(), GL_POINTS, m_material, m_ibo_alloc, m_vbo_alloc);
        }
    }

    void sync() override
    {
        m_vbo_alloc = nullptr;
        m_ibo_alloc = nullptr;

        std::size_t sz = m_particles.active_size();
        if (sz == 0) {
            return;
        }
        std::cout << "allocating storage for " << sz << " particle(s)" << std::endl;
        m_ibo_alloc = m_material.ibo().allocate(sz);
        m_vbo_alloc = m_material.vbo().allocate(sz);

        auto positions = ffe::VBOSlice<Vector3f>(m_vbo_alloc, 0);

        uint16_t *ibo_dest = m_ibo_alloc.get();
        auto iter = m_particles.cbegin();
        for (std::size_t i = 0; i < sz; ++i) {
            assert(iter != m_particles.cend());
            ibo_dest[i] = i;
            positions[i] = Vector3f((*iter)->x - level_width/2,
                                    (*iter)->y - level_height/2,
                                    0);
            ++iter;
        }

        m_vbo_alloc.mark_dirty();
        m_ibo_alloc.mark_dirty();
        m_material.sync_buffers();
    }

};


class PhysicsFlowDebugNode: public ffe::scenegraph::Node
{
public:
    PhysicsFlowDebugNode(ffe::Material &material, NativeLabSim &sim):
        m_material(material),
        m_sim(sim),
        m_ibo_alloc(material.ibo().allocate(level_width*level_height*subdivision_count*2)),
        m_vbo_allocs({
                     material.vbo().allocate(level_width*level_height*subdivision_count*2),
                     material.vbo().allocate(level_width*level_height*subdivision_count*2),
                     material.vbo().allocate(level_width*level_height*subdivision_count*2),
                     material.vbo().allocate(level_width*level_height*subdivision_count*2),
                     material.vbo().allocate(level_width*level_height*subdivision_count*2)
        })
    {
        uint16_t *iout = m_ibo_alloc.get();
        for (uint16_t i = 0; i < level_width*level_height*subdivision_count*2; ++i) {
            *iout++ = i;
        }
        m_ibo_alloc.mark_dirty();
    }

private:
    ffe::Material &m_material;
    NativeLabSim &m_sim;

    ffe::IBOAllocation m_ibo_alloc;
    std::array<ffe::VBOAllocation, subdivision_count> m_vbo_allocs;

public:
    void prepare(ffe::RenderContext&) override
    {

    }

    void render(ffe::RenderContext &context) override
    {
        for (auto &vbo_alloc: m_vbo_allocs) {
            context.render_all(AABB(), GL_LINES, m_material, m_ibo_alloc, vbo_alloc);
        }
    }

    void sync() override
    {
        const float offx = 0.f;
        const float offy = 0.f;

        for (CoordInt y = 0; y < level_height*subdivision_count; ++y) {
            const float yf = (y + 0.5f) / subdivision_count;
            const float yf0 = yf + offy;
            auto posslice = ffe::VBOSlice<Vector3f>(m_vbo_allocs[static_cast<unsigned>(y) / level_height], 0);
            Vector3f *posout = &posslice[(y % level_height)*level_width*subdivision_count*2];
            const LabCell *cell = &m_sim.front_cell_at(0, y);
            for (CoordInt x = 0; x < level_width*subdivision_count; ++x) {
                const float xf = (x + 0.5f) / subdivision_count;
                const float xf0 = xf + offx;
                *posout++ = Vector3f(xf0, yf0, 0);
                Vector2f flow = cell->flow;
                const float flow_magnitude = flow.length();
                if (flow_magnitude > 1e-2f) {
                    flow = (flow / flow_magnitude) * 1e-2;
                }
                *posout++ = Vector3f(xf0 + flow[eX] * 100, yf0 + flow[eY] * 100, 0);

                /* *posout++ = Vector3f(0, 0, 0);
                *posout++ = Vector3f(static_cast<float>(x) / 5.f + offx, static_cast<float>(y) / 5.f + offy, 0); */

                ++cell;
            }
        }

        for (auto &vbo_alloc: m_vbo_allocs) {
            vbo_alloc.mark_dirty();
        }

        m_material.sync_buffers();
    }

};


class BackgroundNode: public ffe::scenegraph::Node
{
public:
    BackgroundNode(ffe::Material &mat):
        m_mat(mat),
        m_vbo_alloc(mat.vbo().allocate(4)),
        m_ibo_alloc(mat.ibo().allocate(4))
    {
        auto positions = ffe::VBOSlice<Vector4f>(m_vbo_alloc, 0);
        auto tc0 = ffe::VBOSlice<Vector2f>(m_vbo_alloc, 1);

        const float z = -0.995f;
        const float w = 4.f;
        positions[0] = Vector4f(-10.f, -10.f, z, w);
        tc0[0] = Vector2f(0.f, 0.f);
        positions[1] = Vector4f(62.f, -10.f, z, w);
        tc0[1] = Vector2f(6.f, 0.f);
        positions[2] = Vector4f(-10.f, 62.f, z, w);
        tc0[2] = Vector2f(0.f, 6.f);
        positions[3] = Vector4f(62.f, 62.f, z, w);
        tc0[3] = Vector2f(6.f, 6.f);

        std::uint16_t *ibo_ptr = m_ibo_alloc.get();
        *ibo_ptr++ = 0;
        *ibo_ptr++ = 2;
        *ibo_ptr++ = 1;
        *ibo_ptr++ = 3;

        m_ibo_alloc.mark_dirty();
        m_vbo_alloc.mark_dirty();
    }

private:
    ffe::Material &m_mat;
    ffe::VBOAllocation m_vbo_alloc;
    ffe::IBOAllocation m_ibo_alloc;

    // Node interface
public:
    void prepare(ffe::RenderContext &context) override;
    void render(ffe::RenderContext &context) override;
    void sync() override;
};

void BackgroundNode::prepare(ffe::RenderContext &context)
{
}

void BackgroundNode::render(ffe::RenderContext &context)
{
    context.render_all(AABB(), GL_TRIANGLE_STRIP, m_mat, m_ibo_alloc, m_vbo_alloc);
}

void BackgroundNode::sync()
{
    m_mat.sync_buffers();
}


class FogNode: public ffe::scenegraph::Node
{
public:
    FogNode(ffe::Material &mat):
        m_mat(mat),
        m_vbo_alloc(mat.vbo().allocate(4)),
        m_ibo_alloc(mat.ibo().allocate(4)),
        m_t(0)
    {
        auto positions = ffe::VBOSlice<Vector4f>(m_vbo_alloc, 0);
        auto tc0 = ffe::VBOSlice<Vector2f>(m_vbo_alloc, 1);

        const float z = 0.005f;
        const float w = 1.f;
        positions[0] = Vector4f(0.f, 0.f, z, w);
        tc0[0] = Vector2f(0.f, 0.f);
        positions[1] = Vector4f(level_width, 0.f, z, w);
        tc0[1] = Vector2f(1.f, 0.f);
        positions[2] = Vector4f(0, level_height, z, w);
        tc0[2] = Vector2f(0.f, 1.f);
        positions[3] = Vector4f(level_width, level_height, z, w);
        tc0[3] = Vector2f(1.f, 1.f);

        std::uint16_t *ibo_ptr = m_ibo_alloc.get();
        *ibo_ptr++ = 0;
        *ibo_ptr++ = 2;
        *ibo_ptr++ = 1;
        *ibo_ptr++ = 3;

        m_ibo_alloc.mark_dirty();
        m_vbo_alloc.mark_dirty();
    }

private:
    ffe::Material &m_mat;
    ffe::VBOAllocation m_vbo_alloc;
    ffe::IBOAllocation m_ibo_alloc;

    float m_t;

    // Node interface
public:
    void prepare(ffe::RenderContext &context) override;
    void render(ffe::RenderContext &context) override;
    void sync() override;

    // Node interface
public:
    void advance(ffe::TimeInterval seconds) override;
};

void FogNode::advance(ffe::TimeInterval seconds)
{
    m_t += seconds;
}

void FogNode::prepare(ffe::RenderContext &context)
{
}

void FogNode::render(ffe::RenderContext &context)
{
    const float t = m_t;
    context.render_all(AABB(), GL_TRIANGLE_STRIP, m_mat, m_ibo_alloc, m_vbo_alloc,
                       [t](ffe::MaterialPass &pass){
        glUniform1f(pass.shader().uniform_location("t"), t);
    });
}

void FogNode::sync()
{
    m_mat.sync_buffers();
}


struct InGameScene
{
    ffe::GLResourceManager m_resources;
    ffe::WindowRenderTarget m_window;
    ffe::SceneGraph m_scenegraph;
    ffe::OrthogonalCamera m_camera;
    ffe::Scene m_scene;
    ffe::RenderGraph m_rendergraph;

    ffe::RenderPass &m_main_pass;

    ffe::Texture2D &m_physics;
    ffe::Texture2D &m_fog_data;
    ffe::Texture2D &m_fog_noise;

    ffe::VBO m_physics_debug_vbo;
    ffe::IBO m_physics_debug_ibo;

    ffe::Material &m_physics_debug_mat;

    ffe::VBO m_physics_particles_debug_vbo;
    ffe::IBO m_physics_particles_debug_ibo;

    ffe::Material &m_physics_particles_debug_mat;

    ffe::VBO m_physics_flow_debug_vbo;
    ffe::IBO m_physics_flow_debug_ibo;

    ffe::Material &m_physics_flow_debug_mat;

    ffe::Material &m_fog_material;

    InGameScene(Level &level):
        m_scene(m_scenegraph, m_camera),
        m_rendergraph(m_scene),
        m_main_pass(m_rendergraph.new_node<ffe::RenderPass>(m_window)),
        m_physics(m_resources.emplace<ffe::Texture2D>(
                      "tex/physics_debug", GL_RGBA32F,
                      level_width*subdivision_count,
                      level_height*subdivision_count)),
        m_fog_data(m_resources.emplace<ffe::Texture2D>(
                       "tex/fog_data", GL_RGBA32F,
                       level_width*subdivision_count,
                       level_height*subdivision_count)),
        m_fog_noise(load_simple_texture(m_resources,
                                        ":/tileset/fog_noise.png",
                                        true)),
        m_physics_debug_vbo(ffe::VBOFormat({ffe::VBOAttribute(3), ffe::VBOAttribute(2)})),
        m_physics_debug_mat(m_resources.emplace<ffe::Material>(
                                "mat/physics_debug",
                                m_physics_debug_vbo,
                                m_physics_debug_ibo
                                )),
        m_physics_particles_debug_vbo(ffe::VBOFormat({ffe::VBOAttribute(3)})),
        m_physics_particles_debug_mat(m_resources.emplace<ffe::Material>(
                                "mat/physics_particles_debug",
                                m_physics_particles_debug_vbo,
                                m_physics_particles_debug_ibo
                                )),
        m_physics_flow_debug_vbo(ffe::VBOFormat({ffe::VBOAttribute(3)})),
        m_physics_flow_debug_mat(m_resources.emplace<ffe::Material>(
                                "mat/physics_flow_debug",
                                m_physics_flow_debug_vbo,
                                m_physics_flow_debug_ibo
                                )),
        m_fog_material(m_resources.emplace<ffe::Material>(
                           "mat/fog",
                           ensure_generic_object_vbo(m_resources),
                           ensure_generic_object_ibo(m_resources)))
    {
        m_camera.controller().set_distance(8.f);
        m_camera.controller().set_rot(Vector2f(0, 0));
        m_camera.controller().set_pos(Vector3f(0, 0, 0));

        m_main_pass.set_clear_mask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        m_main_pass.set_clear_colour(Vector4f(0.f, 0.f, 0.f, 1.0));

        m_physics.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        m_fog_data.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        {
            spp::EvaluationContext ctx(m_resources.shader_library());
            ffe::MaterialPass &pass = m_physics_debug_mat.make_pass_material(m_main_pass);
            bool success = true;

            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/debug/physics.vert"),
                        ctx,
                        GL_VERTEX_SHADER);
            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/debug/physics.frag"),
                        ctx,
                        GL_FRAGMENT_SHADER);

            m_physics_debug_mat.declare_attribute("position", 0);
            m_physics_debug_mat.declare_attribute("tc0", 1);

            success = success && m_physics_debug_mat.link();

            if (!success) {
                throw std::runtime_error("failed to compile or link physics material");
            }

            m_physics_debug_mat.attach_texture("debug_tex", &m_physics);

            pass.set_depth_test(false);
        }

        {
            spp::EvaluationContext ctx(m_resources.shader_library());
            ffe::MaterialPass &pass = m_physics_particles_debug_mat.make_pass_material(m_main_pass);
            bool success = true;

            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/debug/physics_particle.vert"),
                        ctx,
                        GL_VERTEX_SHADER);
            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/debug/physics_particle.frag"),
                        ctx,
                        GL_FRAGMENT_SHADER);

            m_physics_particles_debug_mat.declare_attribute("position", 0);

            success = success && m_physics_particles_debug_mat.link();

            if (!success) {
                throw std::runtime_error("failed to compile or link physics particle material");
            }

            pass.set_depth_test(false);
            pass.set_point_size(5.f);
        }

        {
            spp::EvaluationContext ctx(m_resources.shader_library());
            ffe::MaterialPass &pass = m_physics_flow_debug_mat.make_pass_material(m_main_pass);
            bool success = true;

            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/debug/physics_flow.vert"),
                        ctx,
                        GL_VERTEX_SHADER);
            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/debug/physics_flow.frag"),
                        ctx,
                        GL_FRAGMENT_SHADER);

            m_physics_flow_debug_mat.declare_attribute("position", 0);

            success = success && m_physics_flow_debug_mat.link();

            if (!success) {
                throw std::runtime_error("failed to compile or link physics flow material");
            }

            pass.set_depth_test(false);
        }

        {
            spp::EvaluationContext ctx(m_resources.shader_library());
            ctx.define1ull("GRID_SIZE", level_width * subdivision_count);

            ffe::MaterialPass &pass = m_fog_material.make_pass_material(m_main_pass);
            bool success = true;

            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/objects/object.vert"),
                        ctx,
                        GL_VERTEX_SHADER);
            success = success && pass.shader().attach(
                        m_resources.load_shader_checked(":/shaders/effects/fog.frag"),
                        ctx,
                        GL_FRAGMENT_SHADER);

            m_fog_material.declare_attribute("position", 0);
            m_fog_material.declare_attribute("tc0_in", 1);

            success = success && m_fog_material.link();

            if (!success) {
                throw std::runtime_error("failed to compile or link fog material");
            }

            m_fog_material.attach_texture("fog_data", &m_fog_data);
            m_fog_material.attach_texture("fog_noise", &m_fog_noise);

            pass.set_depth_test(false);
        }

        m_rendergraph.resort();

        create_fallback_material(m_resources, m_main_pass);
        create_simple_material(m_resources, m_main_pass, "safe_wall",
                               ":/tileset/wall_pillar.png");
        create_simple_material(m_resources, m_main_pass, "rock",
                               ":/tileset/rock.png");
        create_simple_material(m_resources, m_main_pass, "player",
                               ":/tileset/player.png").make_pass_material(m_main_pass);
        {
            ffe::Material &mat = create_simple_material(
                        m_resources, m_main_pass, "dirt",
                        ":/tileset/dirt.png",
                        ":/shaders/objects/dirt.frag");
            ffe::ShaderProgram &shader = mat.make_pass_material(m_main_pass).shader();
            shader.bind();
            glUniform1f(
                        shader.uniform_location("texture_scale"),
                        1.f/3.f);
        }

        {
            ffe::Material &bg_mat = create_simple_material(m_resources, m_main_pass, "background", ":/tileset/bg4.png");
            bg_mat.make_pass_material(m_main_pass).set_order(-1);
            m_scenegraph.root().emplace<BackgroundNode>(bg_mat);
        }
        m_scenegraph.root().emplace<LevelView>(m_resources, level, false);
        m_scenegraph.root().emplace<FogNode>(m_fog_material);

        /* m_scenegraph.root().emplace<PhysicsDebugNode>(m_physics_debug_mat);
        m_scenegraph.root().emplace<PhysicsParticlesDebugNode>(
                    m_physics_particles_debug_mat,
                    level.particles()); */
        /* m_scenegraph.root().emplace<PhysicsFlowDebugNode>(
                    m_physics_flow_debug_mat,
                    level.physics()); */
    }

    void update_size(const QSize &new_size)
    {
        if (m_window.width() != new_size.width() ||
                m_window.height() != new_size.height() )
        {
            // resize window
            m_window.set_size(new_size.width(), new_size.height());
        }
    }
};



InGame::InGame(Application &app, QWidget *parent):
    ApplicationMode(app, parent),
    m_ui(new Ui::InGame),
    m_time_buffer(0),
    m_single_step(false),
    m_mouse_pos_x(-1),
    m_mouse_pos_y(-1)
{
    m_ui->setupUi(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

InGame::~InGame()
{
    delete m_ui;
}

Vector2f InGame::widget_pos_to_level_pos(const float x, const float y)
{
    QSize sz = size();
    float min_x, max_x, min_y, max_y;

    if (sz.width() > sz.height()) {
        min_y = 0;
        max_y = sz.height() - 1;
        min_x = (sz.width() - sz.height()) / 2;
        max_x = sz.height() + (sz.width() - sz.height()) / 2 - 1;
    } else {
        min_x = 0;
        max_x = sz.width() - 1;
        min_y = (sz.height() - sz.width()) / 2;
        max_y = sz.width() + (sz.height() - sz.width()) / 2 - 1;
    }

    const float norm_x = (x - min_x) / (max_x - min_x);
    const float norm_y = (y - min_y) / (max_y - min_y);

    const float cell_xf = norm_x * level_width;
    const float cell_yf = norm_y * level_height;

    if (cell_xf < 0 || cell_xf > level_width || cell_yf < 0 || cell_yf > level_height) {
        return Vector2f(NAN, NAN);
    }

    return Vector2f(cell_xf, cell_yf);
}

void InGame::update_probe(const CoordPair phy_probe_pos)
{
    m_ui->probe_pos->setText(
                QString("%1,%2 (%3,%4)").arg(phy_probe_pos.x).arg(phy_probe_pos.y).arg(phy_probe_pos.x / subdivision_count).arg(phy_probe_pos.y / subdivision_count)
                );

    const auto &physics = m_server->state().physics();
    const LabCell *cell = physics.safe_front_cell_at(phy_probe_pos.x, phy_probe_pos.y);
    if (!cell) {
        m_ui->probe_temperature->setText("??");
        m_ui->probe_temperature_celsius->setText("??");
        m_ui->probe_temperature_coefficient->setText("??");
        m_ui->probe_flow_x->setText("??");
        m_ui->probe_flow_y->setText("??");
        m_ui->probe_fog->setText("??");
        m_ui->probe_pressure->setText("??");
        m_ui->probe_heat_energy->setText("??");
        return;
    }

    const LabCellMeta &meta = physics.meta_at(phy_probe_pos.x, phy_probe_pos.y);

    const SimFloat temperature_coefficient = cell->heat_capacity_cache;
    if (meta.blocked) {
        m_ui->probe_pressure->setText("N/A");
        m_ui->probe_flow_x->setText("N/A");
        m_ui->probe_flow_y->setText("N/A");
        m_ui->probe_fog->setText("N/A");
    } else {
        m_ui->probe_fog->setText(QString::number(static_cast<double>(cell->fog_density)));
        m_ui->probe_pressure->setText(QString::number(static_cast<double>(cell->air_pressure)));
        const auto flow = cell->flow;
        m_ui->probe_flow_x->setText(QString::number(static_cast<double>(flow[eX])));
        m_ui->probe_flow_y->setText(QString::number(static_cast<double>(flow[eY])));
    }
    m_ui->probe_temperature_coefficient->setText(QString::number(static_cast<double>(temperature_coefficient)));
    m_ui->probe_heat_energy->setText(QString::number(static_cast<double>(cell->heat_energy)));
    const double temperature = static_cast<double>(cell->heat_energy / temperature_coefficient);
    m_ui->probe_temperature->setText(QString("%1 K").arg(temperature));
    m_ui->probe_temperature_celsius->setText(QString("%1 °C").arg(temperature - KELVIN_TO_CELSIUS));
}

void InGame::advance(ffe::TimeInterval dt)
{
    /* TODO: figure out if this breaks things w.r.t. physics */
    m_scene->m_scenegraph.advance(dt);
}

void InGame::after_gl_sync()
{

}

void InGame::before_gl_sync()
{
    if (!m_scene) {
        m_scene = std::make_unique<InGameScene>(m_server->state());
    }
    m_scene->m_window.set_fbo_id(m_gl_scene->defaultFramebufferObject());

    const QSize size = window()->size() * window()->devicePixelRatioF();
    m_scene->update_size(size);

    m_scene->m_camera.controller().set_pos(Vector3f(
                                               m_player->x + 0.5f,
                                               m_player->y + 0.5f,
                                               0.f
                                               ));

    auto sync_point = m_server->sync_safe_point();

    Vector2f probe_pos = widget_pos_to_level_pos(m_mouse_pos_x, m_mouse_pos_y);
    if (!std::isnan(probe_pos[eX])) {
        update_probe(CoordPair(static_cast<int>(probe_pos[eX] * subdivision_count) ,
                               static_cast<int>(probe_pos[eY] * subdivision_count)));
    }

    /* m_scene->m_physics.bind();
    m_server->state().physics().data_to_gl_texture(); */
    m_scene->m_fog_data.bind();
    m_server->state().physics().fog_data_to_gl_texture();
    m_player->controller() = m_player_controller;

    m_scene->m_camera.sync();
    m_scene->m_scenegraph.sync();
    m_gl_scene->setup_scene(&m_scene->m_rendergraph);
    ffe::raise_last_gl_error();
}


void InGame::activate(QWidget &parent)
{
    ApplicationMode::activate(parent);

    m_server = std::make_unique<Server>();

    /*for (CoordInt y = 0; y < physics.height(); ++y) {
        if (y <= 20*5 || y >= 30*5) {
            physics.set_blocked(20*5, y, true);
            physics.set_blocked(30*5, y, true);
        }
    }*/

    /*m_level->emplace_object<BombObject>(23, 19, default_temperature);
    m_level->emplace_object<BombObject>(24, 19, default_temperature);
    m_level->emplace_object<BombObject>(24, 18, default_temperature);

    m_level->emplace_object<BombObject>(27, 19, default_temperature);
    m_level->emplace_object<BombObject>(26, 19, default_temperature);
    m_level->emplace_object<BombObject>(26, 18, default_temperature);*/

    /* for (CoordInt x = 20; x < 30; ++x) {
        if (x == 25) {
            // m_level->emplace_object<RoundSafeWallObject>(x, 20, default_temperature);
        } else {
            m_level->emplace_object<SafeWallObject>(x, 20, default_temperature);
        }
        m_level->emplace_object<SafeWallObject>(x, 22, default_temperature);
    } */

    /*for (CoordInt y = 0; y < level_height; ++y) {
        m_level->emplace_object<SafeWallObject>(24, y, 1.);
        m_level->emplace_object<SafeWallObject>(26, y, 1.);
        if (y < level_height - 3) {
            m_level->emplace_object<BombObject>(25, y, 1.);
        }
    }*/

    auto sync = m_server->sync_safe_point();
    auto tileset = make_builtin_tileset();
    {
        std::ifstream in("./test.lvl", std::ios::binary);
        if (!in.bad()) {
            load_level(m_server->state(), *tileset, in);
        }
    }

    m_player = m_server->state().emplace_player<PlayerObject>(0, 0);

    m_server->state().physics().reset_unblocked_cells();

    m_advance_conn = connect(
                m_gl_scene,
                &OpenGLScene::advance,
                this,
                &InGame::advance,
                Qt::DirectConnection
                );
    m_before_gl_sync_conn = connect(
                m_gl_scene,
                &OpenGLScene::before_gl_sync,
                this,
                &InGame::before_gl_sync,
                Qt::DirectConnection
                );
    m_after_gl_sync_conn = connect(
                m_gl_scene,
                &OpenGLScene::after_gl_sync,
                this,
                &InGame::after_gl_sync,
                Qt::DirectConnection
                );

    m_gl_scene->update();
    setFocus();
}

void InGame::deactivate()
{
    disconnect(m_after_gl_sync_conn);
    disconnect(m_advance_conn);
    disconnect(m_before_gl_sync_conn);

    m_scene = nullptr;
    m_server = nullptr;

    ApplicationMode::deactivate();
}


void InGame::mouseMoveEvent(QMouseEvent *event)
{
    m_mouse_pos_x = event->x();
    m_mouse_pos_y = event->y();
}

void InGame::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Period) {
        m_single_step = true;
        m_time_buffer += Level::time_slice;
    } else if (event->key() == Qt::Key_Space) {
        m_single_step = !m_single_step;
    } else if (event->key() == Qt::Key_R) {
        /* m_level->physics().wait_for_frame();
        m_level->physics().reset_unblocked_cells(); */
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        /* if (!m_level->get_cell(24, 15)->here) {
            m_level->emplace_object<BombObject>(24, 15, default_temperature);
        } */
    } else if (event->key() == Qt::Key_Up) {
        m_player_controller.action_request = PlayerController::AR_MOVE_UP;
    } else if (event->key() == Qt::Key_Down) {
        m_player_controller.action_request = PlayerController::AR_MOVE_DOWN;
    } else if (event->key() == Qt::Key_Left) {
        m_player_controller.action_request = PlayerController::AR_MOVE_LEFT;
    } else if (event->key() == Qt::Key_Right) {
        m_player_controller.action_request = PlayerController::AR_MOVE_RIGHT;
    } else {
        ApplicationMode::keyPressEvent(event);
    }

}

void InGame::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up ||
            event->key() == Qt::Key_Down ||
            event->key() == Qt::Key_Right ||
            event->key() == Qt::Key_Left)
    {
        m_player_controller.action_request = PlayerController::AR_NONE;
    }
}
