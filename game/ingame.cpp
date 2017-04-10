#include "ingame.hpp"

#include "ui_ingame.h"

#include <ffengine/gl/resource.hpp>

#include "openglscene.hpp"

#include "logic/wall_object.hpp"
#include "logic/fog_object.hpp"
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

        tc0[0] = Vector2f(0, 0);
        tc0[1] = Vector2f(1, 0);
        tc0[2] = Vector2f(0, 1);
        tc0[3] = Vector2f(1, 1);

        m_vbo_alloc.mark_dirty();
        m_material.sync_buffers();
    }

private:
    ffe::Material &m_material;

    ffe::IBOAllocation m_ibo_alloc;
    ffe::VBOAllocation m_vbo_alloc;

public:
    void prepare(ffe::RenderContext &context) override
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

    InGameScene():
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
                                ))
    {
        m_camera.controller().set_distance(50.f);
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
                throw std::runtime_error("failed to compile or link AABB material");
            }

            m_physics_debug_mat.attach_texture("debug_tex", &m_physics);
        }

        m_rendergraph.resort();

        m_scenegraph.root().emplace<PhysicsDebugNode>(m_physics_debug_mat);
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
    m_time_buffer(0)
{
    m_ui->setupUi(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

InGame::~InGame()
{
    delete m_ui;
}

void InGame::advance(ffe::TimeInterval dt)
{
    const ffe::TimeInterval time_slice = Level::time_slice;
    m_time_buffer += dt;
    while (m_time_buffer >= time_slice) {
        m_level->update();
        m_time_buffer -= time_slice;
    }
}

void InGame::after_gl_sync()
{

}

void InGame::before_gl_sync()
{
    if (!m_scene) {
        m_scene = std::make_unique<InGameScene>();
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

    NativeLabSim &physics = m_level->physics();

    /*for (CoordInt y = 0; y < physics.height(); ++y) {
        if (y <= 20*5 || y >= 30*5) {
            physics.set_blocked(20*5, y, true);
            physics.set_blocked(30*5, y, true);
        }
    }*/

    /*for (CoordInt x = 20; x < 30; ++x) {
        m_level->emplace_object<SafeWallObject>(x, 20, 1.0);
        m_level->emplace_object<SafeWallObject>(x, 22, 1.0);
    }*/

    m_level->emplace_object<FogObject>(30, 21, 1.0, 0.6, 1.0);
    /*m_level->emplace_object<HorizFanObject>(19, 21, 1.0, -1.0);*/
    m_level->emplace_object<HorizFanObject>(29, 21, 1.0, -1.0);

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
