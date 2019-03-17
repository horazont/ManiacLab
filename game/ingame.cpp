#include "ingame.hpp"

#include "ui_ingame.h"

#include <ffengine/gl/resource.hpp>

#include <QKeyEvent>

#include "openglscene.hpp"

#include "logic/wall_object.hpp"
#include "logic/fog_object.hpp"
#include "logic/bomb_object.hpp"
#include "logic/fan_object.hpp"


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

        positions[0] = Vector3f(-level_width/2, -level_height/2, 0);
        positions[1] = Vector3f(level_width/2, -level_height/2, 0);
        positions[2] = Vector3f(-level_width/2, level_height/2, 0);
        positions[3] = Vector3f(level_width/2, level_height/2, 0);

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
        const float offx = -level_width / 2.f;
        const float offy = -level_height / 2.f;

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

    ffe::VBO m_physics_debug_vbo;
    ffe::IBO m_physics_debug_ibo;

    ffe::Material &m_physics_debug_mat;

    ffe::VBO m_physics_particles_debug_vbo;
    ffe::IBO m_physics_particles_debug_ibo;

    ffe::Material &m_physics_particles_debug_mat;

    ffe::VBO m_physics_flow_debug_vbo;
    ffe::IBO m_physics_flow_debug_ibo;

    ffe::Material &m_physics_flow_debug_mat;

    InGameScene(Level &level):
        m_scene(m_scenegraph, m_camera),
        m_rendergraph(m_scene),
        m_main_pass(m_rendergraph.new_node<ffe::RenderPass>(m_window)),
        m_physics(m_resources.emplace<ffe::Texture2D>(
                      "tex/physics_debug", GL_RGBA32F,
                      level_width*subdivision_count,
                      level_height*subdivision_count)),
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
                                ))
    {
        m_camera.controller().set_distance(level_width);
        m_camera.controller().set_rot(Vector2f(0, 0));
        m_camera.controller().set_pos(Vector3f(0, 0, 0));

        m_main_pass.set_clear_mask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        m_main_pass.set_clear_colour(Vector4f(0.5, 0.4, 0.3, 1.0));

        m_physics.bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

        m_rendergraph.resort();

        m_scenegraph.root().emplace<PhysicsDebugNode>(m_physics_debug_mat);
        m_scenegraph.root().emplace<PhysicsParticlesDebugNode>(
                    m_physics_particles_debug_mat,
                    level.particles());
        m_scenegraph.root().emplace<PhysicsFlowDebugNode>(
                    m_physics_flow_debug_mat,
                    level.physics());
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

    const auto &physics = m_level->physics();
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

    float temperature_coefficient = 0;
    if (meta.blocked) {
        m_ui->probe_pressure->setText("N/A");
        m_ui->probe_flow_x->setText("N/A");
        m_ui->probe_flow_y->setText("N/A");
        m_ui->probe_fog->setText("N/A");
        temperature_coefficient = meta.obj->info.temp_coefficient;
    } else {
        temperature_coefficient = cell->air_pressure;
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
    const ffe::TimeInterval time_slice = Level::time_slice;
    if (!m_single_step) {
        m_time_buffer += dt;
    }
    while (m_time_buffer >= time_slice) {
        m_level->update();
        m_time_buffer -= time_slice;
    }

    Vector2f probe_pos = widget_pos_to_level_pos(m_mouse_pos_x, m_mouse_pos_y);
    if (!std::isnan(probe_pos[eX])) {
        update_probe(CoordPair(static_cast<int>(probe_pos[eX] * subdivision_count) ,
                               static_cast<int>(probe_pos[eY] * subdivision_count)));
    }
}

void InGame::after_gl_sync()
{

}

void InGame::before_gl_sync()
{
    if (!m_scene) {
        m_scene = std::make_unique<InGameScene>(*m_level);
    }
    m_scene->m_window.set_fbo_id(m_gl_scene->defaultFramebufferObject());

    const QSize size = window()->size() * window()->devicePixelRatioF();
    m_scene->update_size(size);

    m_scene->m_physics.bind();
    m_level->physics().data_to_gl_texture();

    m_scene->m_camera.sync();
    m_scene->m_scenegraph.sync();
    m_gl_scene->setup_scene(&m_scene->m_rendergraph);
    ffe::raise_last_gl_error();
}


void InGame::activate(QWidget &parent)
{
    ApplicationMode::activate(parent);

    m_level = std::make_unique<Level>(level_width, level_height);

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

    for (CoordInt x = 20; x < 30; ++x) {
        if (x == 25) {
            // m_level->emplace_object<RoundSafeWallObject>(x, 20, default_temperature);
        } else {
            m_level->emplace_object<SafeWallObject>(x, 20, default_temperature);
        }
        m_level->emplace_object<SafeWallObject>(x, 22, default_temperature);
    }

    /*for (CoordInt y = 0; y < level_height; ++y) {
        m_level->emplace_object<SafeWallObject>(24, y, 1.);
        m_level->emplace_object<SafeWallObject>(26, y, 1.);
        if (y < level_height - 3) {
            m_level->emplace_object<BombObject>(25, y, 1.);
        }
    }*/

    for (CoordInt x = 0; x < level_width; ++x) {
        m_level->emplace_object<SafeWallObject>(x, 0, default_temperature);
        m_level->emplace_object<SafeWallObject>(x, level_height-1, default_temperature);
    }

    for (CoordInt y = 0; y < level_height; ++y) {
        m_level->emplace_object<SafeWallObject>(0, y, default_temperature);
        m_level->emplace_object<SafeWallObject>(level_width-1, y, default_temperature);
    }

    /* m_level->emplace_object<FogObject>(30, 21, default_temperature, 0.6, 1.0);
    m_level->emplace_object<VertFanObject>(24, 20, default_temperature, 1.f, 0.8f);
    m_level->emplace_object<VertFanObject>(24, 22, default_temperature, 1.f, 0.8f);
    m_level->emplace_object<HorizFanObject>(19, 21, default_temperature, -3.0, 0.8);
    m_level->emplace_object<HorizFanObject>(29, 21, default_temperature, -3.0, 0.8);

    for (CoordInt y = 16; y < 26; ++y) {
        if (y == 21) {
            continue;
        }
        m_level->emplace_object<SafeWallObject>(19, y, default_temperature);
        m_level->emplace_object<SafeWallObject>(29, y, default_temperature);
    }

    for (CoordInt x = 20; x < 29; ++x) {
        if (x == 24) {
            continue;
        }
        m_level->emplace_object<SafeWallObject>(x, 16, default_temperature);
    }

    {
        const float heater_temp = 1.2f;
        const float cooler_temp = 0.5f;
        const float heater_rate = 5e-4f;
        const float cooler_rate = 5e-4f;
        m_level->emplace_object<SafeWallObject>(25, 45, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(heater_rate).set_heater_target_temperature(heater_temp);
        m_level->emplace_object<SafeWallObject>(26, 45, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(heater_rate).set_heater_target_temperature(heater_temp);

        m_level->emplace_object<SafeWallObject>(25, 47, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(cooler_rate).set_heater_target_temperature(cooler_temp);
        m_level->emplace_object<SafeWallObject>(26, 47, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(cooler_rate).set_heater_target_temperature(cooler_temp);
    } */

    {
        /* const float heater_temp = 400;
        const float heater_rate = 5e-15f;
        m_level->emplace_object<SafeWallObject>(25, 20, heater_temp)->set_heater_enabled(true).set_heater_energy_rate(heater_rate).set_heater_target_temperature(heater_temp); */
        /* m_level->emplace_object<SafeWallObject>(26, 20, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(heater_rate).set_heater_target_temperature(heater_temp); */

        /*const float cooler_temp = 0.5f;
        const float cooler_rate = 5e-4f;
        m_level->emplace_object<SafeWallObject>(25, 20, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(cooler_rate).set_heater_target_temperature(cooler_temp);
        m_level->emplace_object<SafeWallObject>(26, 20, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(cooler_rate).set_heater_target_temperature(cooler_temp); */
    }

    /* m_level->emplace_object<BombObject>(25, 19, default_temperature);
    m_level->emplace_object<HorizFanObject>(27, 19, default_temperature, -0.6f, 0.4f);

    for (CoordInt y = 15; y < 25; ++y) {
        const float cooler_temp = 270;
        const float cooler_rate = 5e-15f;

        m_level->emplace_object<SafeWallObject>(29, y, cooler_temp)->set_heater_enabled(true).set_heater_energy_rate(cooler_rate).set_heater_target_temperature(cooler_temp);
        if (y == 19) {
            continue;
        }
        m_level->emplace_object<SafeWallObject>(27, y, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(1e-6f).set_heater_target_temperature(1.f);
    }*/

    /* m_level->emplace_object<FogObject>(25, 25, default_temperature, 0.6, 1.0);
    m_level->emplace_object<FogObject>(25, 26, default_temperature, 0.6, 1.0);
    m_level->emplace_object<FogObject>(26, 26, default_temperature, 0.6, 1.0);
    m_level->emplace_object<FogObject>(26, 25, default_temperature, 0.6, 1.0); */

    /* m_level->emplace_object<BombObject>(25, 19, default_temperature);
    m_level->emplace_object<RoundSafeWallObject>(25, 20, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(5e-4).set_heater_target_temperature(1.6);
    m_level->emplace_object<SafeWallObject>(24, 18, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(3e-4).set_heater_target_temperature(0.8);
    m_level->emplace_object<SafeWallObject>(26, 18, default_temperature)->set_heater_enabled(true).set_heater_energy_rate(5e-4).set_heater_target_temperature(1.4); */

    m_level->physics().reset_unblocked_cells();

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
    m_level = nullptr;

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
        m_level->physics().wait_for_frame();
        m_level->physics().reset_unblocked_cells();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (!m_level->get_cell(18, 18)->here) {
            m_level->emplace_object<BombObject>(25, 15, default_temperature);
        }
    } else {
        ApplicationMode::keyPressEvent(event);
    }

}
