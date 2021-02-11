#include "scene.hpp"

#include <QImage>
#include <QPainter>

#include <ffengine/render/fullscreenquad.hpp>

#include "materials.hpp"

static float lightning_effect(const float t) {
    if (t < 0) {
        return 0;
    }

    return std::exp(-t * 15.f) + std::max(0.f, (-sqr((t-0.06f)*30.f) + 0.4f));
}


const std::array<Vector4f, 4> Quad::vertices = {{Vector4f(-1.f, 1.f, 0.f, 1.f),
                                                 Vector4f(1.f, 1.f, 0.f, 1.f),
                                                 Vector4f(-1.f, -1.f, 0.f, 1.f),
                                                 Vector4f(1.f, -1.f, 0.f, 1.f)}};

Quad::Quad(ffe::Material &material):
    m_material(material),
    m_vbo_alloc(material.vbo().allocate(4)),
    m_ibo_alloc(material.ibo().allocate(4)),
    m_transform(Identity)
{
    uint16_t *dest = m_ibo_alloc.get();
    *dest++ = 0;
    *dest++ = 2;
    *dest++ = 1;
    *dest++ = 3;

    m_ibo_alloc.mark_dirty();

    auto tc0 = ffe::VBOSlice<Vector2f>(m_vbo_alloc, 1);

    tc0[0] = Vector2f(0, 0);
    tc0[1] = Vector2f(1, 0);
    tc0[2] = Vector2f(0, 1);
    tc0[3] = Vector2f(1, 1);

}

void Quad::prepare(ffe::RenderContext &context)
{
}

void Quad::render(ffe::RenderContext &context)
{
    context.render_all(AABB(), GL_TRIANGLE_STRIP, m_material, m_ibo_alloc, m_vbo_alloc);
}

void Quad::sync()
{
    auto pos = ffe::VBOSlice<Vector4f>(m_vbo_alloc, 0);

    pos[0] = m_transform * vertices[0];
    pos[1] = m_transform * vertices[1];
    pos[2] = m_transform * vertices[2];
    pos[3] = m_transform * vertices[3];

    m_vbo_alloc.mark_dirty();
    m_material.sync_buffers();
}


IntroScene::IntroScene():
    m_scene(m_scenegraph, m_camera),
    m_rendergraph(m_scene),
    m_main_pass(m_rendergraph.new_node<ffe::RenderPass>(m_window))
{
    m_main_pass.set_clear_colour(Vector4f(0.2, 0.3, 0.4, 1.0));
    m_main_pass.set_clear_mask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    m_camera.set_fovy(60.f / 180.f * M_PI);
    m_camera.set_znear(1.f);
    m_camera.set_zfar(100.f);

    m_rendergraph.resort();
}

IntroScene::~IntroScene()
{

}

void IntroScene::set_fbo_id(GLuint framebuffer_object_id)
{
    m_window.set_fbo_id(framebuffer_object_id);
}

void IntroScene::update_size(const QSize &new_size)
{
    if (m_window.width() != new_size.width() ||
            m_window.height() != new_size.height() )
    {
        // resize window
        m_window.set_size(new_size.width(), new_size.height());
    }
}

bool IntroScene::advance(ffe::TimeInterval)
{
    return true;
}

void IntroScene::sync()
{
    m_camera.sync();
    m_scenegraph.sync();
}

/* INTRO SCENE 1 */

IntroScene1::IntroScene1():
    IntroScene(),
    m_rain_textures{{
                    &m_resources.emplace<ffe::Texture2D>("intro/rain_texture_0",
                                                         GL_R8,
                                                         256, 256,
                                                         GL_RED,
                                                         GL_UNSIGNED_BYTE),
                    &m_resources.emplace<ffe::Texture2D>("intro/rain_texture_1",
                                                         GL_R8,
                                                         256, 256,
                                                         GL_RED,
                                                         GL_UNSIGNED_BYTE),
                    &m_resources.emplace<ffe::Texture2D>("intro/rain_texture_2",
                                                         GL_R8,
                                                         256, 256,
                                                         GL_RED,
                                                         GL_UNSIGNED_BYTE),
                    }},
    m_t_abs(0.f),
    m_t_movement(0.f),
    m_lightning_triggered(false)
{
    m_camera.controller().set_pos(Vector3f(0.f, 0.f, 0.f));
    m_camera.controller().set_distance(10.f);

    {
        ffe::Material &mat = create_simple_material(m_resources,
                                                    m_main_pass,
                                                    "intro_scene_0_img_0",
                                                    ":/intro/s0/i0.png",
                                                    ":/shaders/intro/mixable.frag",
                                                    ":/shaders/intro/simple.vert");
        ffe::MaterialPass &pass = mat.make_pass_material(m_main_pass);
        pass.set_order(0);
        m_scenegraph.root().emplace<Quad>(mat).transform() = translation4(Vector3f(0, 0, -20.f)) * scale4(Vector3f(30.f, 30.f, 30.f));

        m_background_shader = &pass.shader();
    }

    {
        ffe::Material &mat = create_simple_material(m_resources,
                                                    m_main_pass,
                                                    "intro_scene_0_img_1",
                                                    ":/intro/s0/i1.png",
                                                    ":/shaders/intro/mixable.frag",
                                                    ":/shaders/intro/simple.vert");
        ffe::MaterialPass &pass = mat.make_pass_material(m_main_pass);
        pass.set_order(2);
        const float size = 15.f;
        m_scenegraph.root().emplace<Quad>(mat).transform() = translation4(Vector3f(-1.f, -4.5f, 0.f)) * scale4(Vector3f(size, size, size));

        m_wall_shader = &pass.shader();
    }

    static const std::array<Vector2f, n_rain_layers> offsets = {{Vector2f(-1.f, -2.f),
                                                                Vector2f(0.4f, -0.2f),
                                                                Vector2f(-0.8f, 1.8f)}};

    for (unsigned int i = 0; i < m_rain_textures.size(); ++i) {
        const std::string mat_name = "intro/rain_material_" + std::to_string(i);
        ffe::Material &mat = m_resources.emplace<ffe::Material>(
                    mat_name,
                    ensure_generic_object_vbo(m_resources),
                    ensure_generic_object_ibo(m_resources)
                    );
        spp::EvaluationContext ctx(m_resources.shader_library());
        ffe::MaterialPass &pass = mat.make_pass_material(m_main_pass);

        bool success = true;

        success = success && pass.shader().attach(
                    m_resources.load_shader_checked(":/shaders/intro/simple.vert"),
                    ctx,
                    GL_VERTEX_SHADER);
        success = success && pass.shader().attach(
                    m_resources.load_shader_checked(":/shaders/intro/rain-overlay.frag"),
                    ctx,
                    GL_FRAGMENT_SHADER);

        mat.declare_attribute("position", 0);
        mat.declare_attribute("tc0_in", 1);

        success = success && pass.link();

        if (!success) {
            throw std::runtime_error("failed to compile or link rain material");
        }

        ffe::Texture2D *texture = m_rain_textures[i];
        texture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        mat.attach_texture("diffuse", texture);

        pass.shader().bind();
        glUniform1f(pass.shader().uniform_location("scale"), 6.f);

        pass.set_depth_test(false);
        pass.set_order(4 + i);

        const float size = 15.f / (1.f - static_cast<float>(i) / 4.f);
        const Vector2f offset = offsets[i];
        m_scenegraph.root().emplace<Quad>(mat).transform() = translation4(Vector3f(offset[eX], offset[eY], static_cast<float>(i) * 5.f + 3.f )) * scale4(Vector3f(size, size, size));
    }

    {
        const std::string texture_name = ":/intro/s0/lightning-mask.png";
        ffe::Material &mat = create_simple_material(m_resources,
                                                    m_main_pass,
                                                    "intro_scene_0_lighting_effect",
                                                    texture_name,
                                                    ":/shaders/intro/lightning-effect.frag",
                                                    ":/shaders/intro/simple.vert");
        ffe::MaterialPass &pass = mat.make_pass_material(m_main_pass);
        pass.set_order(1);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = 12.f;
        m_scenegraph.root().emplace<Quad>(mat).transform() = translation4(Vector3f(-1.f, -8.5f, -17.f)) * scale4(Vector3f(aspect_ratio*size, size, size));

        m_lightning_shader = &pass.shader();
    }

    {
        auto &lightcone = m_scenegraph.root().emplace<ffe::FullScreenQuadNode>();
        spp::EvaluationContext ctx(m_resources.shader_library());
        ffe::MaterialPass &pass = lightcone.make_pass_material(m_main_pass);
        pass.set_order(10000);

        bool success = true;

        success = success && pass.shader().attach(
                    m_resources.load_shader_checked(":/shaders/effects/fullscreenquad.vert"),
                    ctx,
                    GL_VERTEX_SHADER);
        success = success && pass.shader().attach(
                    m_resources.load_shader_checked(":/shaders/effects/lightcone.frag"),
                    ctx,
                    GL_FRAGMENT_SHADER);

        success = success && pass.link();

        if (!success) {
            throw std::runtime_error("failed to compile or link ghost shader material");
        }

        m_lightcone_shader = &pass.shader();
    }
}

bool IntroScene1::advance(ffe::TimeInterval dt)
{
    m_t_abs = m_t_abs + dt;
    m_t_movement = clamp(m_t_abs * 1.2f, 0.f, 15.f);

    if (!m_lightning_triggered && m_t_abs >= lightning_t) {
        m_t_lightning = m_t_abs;
        m_lightning_triggered = true;
    }

    m_camera.controller().set_distance(15.f - m_t_movement * 2.f / 3.f);
    m_camera.controller().set_pos(Vector3f(0.f, -m_t_movement * 2.f / 3.f, 0.f));

    return m_t_abs > 15.f;
}

static void generate_rain_texture(const GLuint width,
                                  const GLuint height,
                                  std::mt19937 &rng)
{
    QImage img(width, height, QImage::Format_Grayscale8);

    {
        QPainter painter(&img);
        painter.fillRect(0, 0, width, height, Qt::black);

        painter.setPen(QColor(255, 255, 255, 31));

        const unsigned int nstrokes = std::uniform_int_distribution<unsigned>(2, 5)(rng);
        std::normal_distribution<float> stroke_length_gen(40.f, 12.f);
        std::normal_distribution<float> angle_gen(0.f, M_PIf32 / 60.f);
        std::uniform_int_distribution<unsigned> origin_x_gen(0, width);
        std::uniform_int_distribution<unsigned> origin_y_gen(0, height);
        for (unsigned int i = 0; i < nstrokes; ++i) {
            const float length = stroke_length_gen(rng);
            const float angle = angle_gen(rng);
            const float x0 = origin_x_gen(rng);
            const float y0 = origin_y_gen(rng);

            const QPointF p0(x0, y0);
            const QPointF p1(p0 + QPointF(std::sin(angle) * length,
                                          std::cos(angle) * length));
            painter.drawLine(p0, p1);
        }
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0,
                    width, height,
                    GL_RED, GL_UNSIGNED_BYTE,
                    img.scanLine(0));
}

void IntroScene1::sync()
{
    std::mt19937 rng(63157 + std::round(std::sin(m_t_abs) * 10000000.f));
    for (ffe::Texture2D *rain_tex: m_rain_textures) {
        rain_tex->bind();
        generate_rain_texture(rain_tex->width(),
                              rain_tex->height(),
                              rng);
    }

    m_lightcone_shader->bind();
    glUniform2f(m_lightcone_shader->uniform_location("position"), 0.f, m_t_movement * 0.2f / 15.f);
    const float aspect_ratio = static_cast<float>(m_window.width()) / m_window.height();

    const float lightcone_scale = 1.f / (m_t_movement / 30.f + 1.5f);
    const float alpha = (m_lightning_triggered ? lightning_effect(m_t_abs - m_t_lightning) : 0.f);
    glUniform2f(m_lightcone_shader->uniform_location("scale"), aspect_ratio * lightcone_scale, lightcone_scale);
    glUniform3f(m_lightcone_shader->uniform_location("edge_colour"),
                alpha, alpha, alpha);

    m_lightning_shader->bind();
    glUniform1f(m_lightning_shader->uniform_location("alpha"), alpha);

    m_wall_shader->bind();
    glUniform4f(m_wall_shader->uniform_location("mix_colour"), alpha, alpha, alpha, 0.f);

    m_background_shader->bind();
    glUniform4f(m_background_shader->uniform_location("mix_colour"), 0.f, 0.f, 0.f, alpha / 2.f);

    IntroScene::sync();
}

IntroScene2::IntroScene2():
    IntroScene(),
    m_t_abs(0.f),
    m_t_movement(0.f)
{
    m_camera.controller().set_pos(Vector3f(0.f, 0.f, 0.f));
    m_camera.controller().set_distance(10.f);

    {
        const std::string texture_name = ":/intro/s1/bg.png";
        ffe::Material &mat = create_intro_material(m_resources,
                                                   m_main_pass,
                                                   "intro_scene_1_bg",
                                                   texture_name);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = 36.f;
        mat.make_pass_material(m_main_pass).set_order(0);
        m_scenegraph.root().emplace<Quad>(mat).transform() =
                translation4(Vector3f(0, 0, -40.f)) *
                scale4(Vector3f(size*aspect_ratio, size, size));
    }

    const float cat_scale = 7.f;
    const float part_scale = cat_scale / 500.f;
    const float rcat_scale = part_scale * 726.f;
    const Vector3f lcat_torso_pos(-part_scale * 2.f * 330.f, -part_scale * 285.f * 2.f, -16.f);
    const Vector3f rcat_torso_pos(lcat_torso_pos + Vector3f(part_scale * 2.f * 981.f, part_scale * 110.f * 2.f, 0.f));

    {
        const std::string texture_name = ":/intro/s1/lcat-torso.png";
        ffe::Material &mat = create_intro_material(m_resources,
                                                   m_main_pass,
                                                   "intro_scene_1_lcat_torso",
                                                   texture_name);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = cat_scale;
        mat.make_pass_material(m_main_pass).set_order(0);
        m_scenegraph.root().emplace<Quad>(mat).transform() =
                translation4(lcat_torso_pos) *
                scale4(Vector3f(size*aspect_ratio, size, size));
    }

    {
        const std::string texture_name = ":/intro/s1/rcat-torso.png";
        ffe::Material &mat = create_intro_material(m_resources,
                                                   m_main_pass,
                                                   "intro_scene_1_rcat_torso",
                                                   texture_name);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = rcat_scale;
        mat.make_pass_material(m_main_pass).set_order(0);
        m_scenegraph.root().emplace<Quad>(mat).transform() =
                translation4(rcat_torso_pos) *
                scale4(Vector3f(size*aspect_ratio, size, size));
    }

    {
        const std::string texture_name = ":/intro/s1/lcat-head.png";
        ffe::Material &mat = create_intro_material(m_resources,
                                                   m_main_pass,
                                                   "intro_scene_1_lcat_head",
                                                   texture_name);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = cat_scale * 1.278f;
        mat.make_pass_material(m_main_pass).set_order(0);
        const Vector3f com_offset = Vector3f(-1.f, 72.f, 0.f) * part_scale * 2.f;
        m_lhead_transform_rhs =
                translation4(com_offset) *
                scale4(Vector3f(size*aspect_ratio, size, size))
                ;
        m_lhead_transform_lhs =
                translation4(lcat_torso_pos + Vector3f(part_scale * 143.f * 2.f, part_scale * 292.f * 2.f, 0.f) - com_offset);
        m_lhead_transform = &m_scenegraph.root().emplace<Quad>(mat).transform();
    }

    {
        const std::string texture_name = ":/intro/s1/rcat-head.png";
        ffe::Material &mat = create_intro_material(m_resources,
                                                   m_main_pass,
                                                   "intro_scene_1_rcat_head",
                                                   texture_name);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = rcat_scale;
        mat.make_pass_material(m_main_pass).set_order(0);
        const Vector3f com_offset = Vector3f(9.5f, 93.f, 0.f) * part_scale * 2.f;
        m_rhead_transform_rhs =
                translation4(com_offset) *
                scale4(Vector3f(size*aspect_ratio, size, size))
                ;
        m_rhead_transform_lhs =
                translation4(rcat_torso_pos + Vector3f(-part_scale * 330.f * 2.f, part_scale * 285.f * 2.f, 0.f) - com_offset);
        m_rhead_transform = &m_scenegraph.root().emplace<Quad>(mat).transform();
    }

    {
        const std::string texture_name = ":/intro/s1/lcat-tail.png";
        ffe::Material &mat = create_intro_material(m_resources,
                                                   m_main_pass,
                                                   "intro_scene_1_lcat_tail",
                                                   texture_name);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = cat_scale * 1.432f;
        mat.make_pass_material(m_main_pass).set_order(0);

        const Vector3f com_offset = Vector3f(-38.f, 384.f, 0.f) * part_scale * 2.f;
        m_ltail_transform_rhs =
                translation4(com_offset) *
                scale4(Vector3f(size*aspect_ratio, size, size));
        m_ltail_transform_lhs =
                translation4(lcat_torso_pos + Vector3f(-part_scale * 467.f * 2.f, part_scale * 100.f * 2.f, 0.f) - com_offset);
        m_ltail_transform = &m_scenegraph.root().emplace<Quad>(mat).transform();
    }

    {
        const std::string texture_name = ":/intro/s1/rcat-tail.png";
        ffe::Material &mat = create_intro_material(m_resources,
                                                   m_main_pass,
                                                   "intro_scene_1_rcat_tail",
                                                   texture_name);
        ffe::Texture2D &tex = *m_resources.get_safe<ffe::Texture2D>(texture_name);
        const float aspect_ratio = static_cast<float>(tex.width()) / tex.height();
        const float size = cat_scale * 1.368f;
        mat.make_pass_material(m_main_pass).set_order(0);

        const Vector3f com_offset = Vector3f(-173.f, 270.f, 0.f) * part_scale * 2.f;
        m_rtail_transform_rhs =
                translation4(com_offset) *
                scale4(Vector3f(size*aspect_ratio, size, size));
        m_rtail_transform_lhs =
                translation4(rcat_torso_pos + Vector3f(part_scale * 153.f * 2.f, part_scale * 245.f * 2.f, 0.f) - com_offset);
        m_rtail_transform = &m_scenegraph.root().emplace<Quad>(mat).transform();
    }
}

bool IntroScene2::advance(ffe::TimeInterval dt)
{
    m_t_abs = m_t_abs + dt / 5.f;
    m_t_movement = clamp(m_t_abs * 1.2f, 0.f, 1.f);

    *m_rhead_transform =
            m_rhead_transform_lhs *
            rotation4(eZ, m_t_movement * M_PI_2 * 0.1f) *
            m_rhead_transform_rhs;
    *m_lhead_transform =
            m_lhead_transform_lhs *
            translation4(Vector3f(-0.2f, -0.3f, 0.f) * m_t_movement) *
            rotation4(eZ, -m_t_movement * M_PI_2 * 0.1f) *
            m_lhead_transform_rhs;
    *m_rtail_transform =
            m_rtail_transform_lhs *
            rotation4(eZ, m_t_movement * M_PI_4 * 0.04f) *
            m_rtail_transform_rhs;
    *m_ltail_transform =
            m_ltail_transform_lhs *
            rotation4(eZ, -m_t_movement * M_PI_4 * 0.04f) *
            m_ltail_transform_rhs;

    m_camera.controller().set_distance(10.f - m_t_movement * 1.f);

    return m_t_abs > 1.f;
}
