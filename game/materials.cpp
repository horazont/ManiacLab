#include "materials.hpp"

#include <QImage>

static const char *const object_vbo_name = "mat/object/common/vbo";
static const char *const object_ibo_name = "mat/object/common/ibo";

ffe::VBO &ensure_generic_object_vbo(ffe::GLResourceManager &resources)
{
    ffe::VBO *result = resources.get_safe<ffe::VBO>(object_vbo_name);
    if (result) {
        return *result;
    }

    return resources.emplace<ffe::VBO>(
                object_vbo_name,
                ffe::VBOFormat({ffe::VBOAttribute(4), ffe::VBOAttribute(2)})
                );
}

ffe::IBO &ensure_generic_object_ibo(ffe::GLResourceManager &resources)
{
    ffe::IBO *result = resources.get_safe<ffe::IBO>(object_ibo_name);
    if (result) {
        return *result;
    }

    return resources.emplace<ffe::IBO>(object_ibo_name);
}

/*void load_image_to_texture(const QString &url,
                           const GLenum target,
                           bool setup_mipmap)
{
    QImage texture = QImage(url);
    texture = texture.convertToFormat(QImage::Format_ARGB32);

    uint8_t *pixbase = texture.bits();
    for (int i = 0; i < texture.width()*texture.height(); i++)
    {
        const uint8_t A = pixbase[3];
        const uint8_t R = pixbase[2];
        const uint8_t G = pixbase[1];
        const uint8_t B = pixbase[0];

        pixbase[0] = R;
        pixbase[1] = G;
        pixbase[2] = B;
        pixbase[3] = A;

        pixbase += 4;
    }

    if (setup_mipmap) {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexSubImage2D(target, 0,
                    0, 0,
                    texture.width(), texture.height(),
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    texture.bits());
    if (setup_mipmap) {
        glGenerateMipmap(target);
    }
}*/

ffe::Texture2D &load_simple_texture(ffe::GLResourceManager &resources,
                                    const std::string &path,
                                    bool setup_mipmap)
{
    {
        ffe::Texture2D *potential_result = resources.get_safe<ffe::Texture2D>(path);
        if (potential_result) {
            return *potential_result;
        }
    }

    QImage texture = QImage(QString::fromStdString(path));
    if (texture.isNull()) {
        throw std::runtime_error("failed to load image from "+path);
    }

    texture = texture.convertToFormat(QImage::Format_ARGB32);

    uint8_t *pixbase = texture.bits();
    /* convert ARGB to RGBA */
    for (int i = 0; i < texture.width()*texture.height(); i++)
    {
        const uint8_t A = pixbase[3];
        const uint8_t R = pixbase[2];
        const uint8_t G = pixbase[1];
        const uint8_t B = pixbase[0];

        pixbase[0] = R;
        pixbase[1] = G;
        pixbase[2] = B;
        pixbase[3] = A;

        pixbase += 4;
    }

    ffe::Texture2D &result = resources.emplace<ffe::Texture2D>(
                path,
                GL_RGBA8, texture.width(), texture.height(),
                GL_RGBA, GL_UNSIGNED_BYTE);

    ffe::raise_last_gl_error();

    result.bind();

    ffe::raise_last_gl_error();

    if (setup_mipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    ffe::raise_last_gl_error();

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0,
                    texture.width(), texture.height(),
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    texture.bits());

    ffe::raise_last_gl_error();

    if (setup_mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    ffe::raise_last_gl_error();

    return result;
}

ffe::Material &create_fallback_material(ffe::GLResourceManager &resources,
                                        ffe::RenderPass &solid_pass)
{
    auto &fallback_mat = resources.emplace<ffe::Material>(
                "mat/object/fallback",
                ensure_generic_object_vbo(resources),
                ensure_generic_object_ibo(resources));
    spp::EvaluationContext ctx(resources.shader_library());
    ffe::MaterialPass &pass = fallback_mat.make_pass_material(solid_pass);

    bool success = true;

    success = success && pass.shader().attach(
                resources.load_shader_checked(":/shaders/objects/object.vert"),
                ctx,
                GL_VERTEX_SHADER);
    success = success && pass.shader().attach(
                resources.load_shader_checked(":/shaders/objects/fallback.frag"),
                ctx,
                GL_FRAGMENT_SHADER);

    fallback_mat.declare_attribute("position", 0);
    fallback_mat.declare_attribute("tc0_in", 1);

    success = success && pass.link();

    if (!success) {
        throw std::runtime_error("failed to compile or link fallback shader material");
    }

    return fallback_mat;
}

ffe::Material &create_simple_material(ffe::GLResourceManager &resources,
                                      ffe::RenderPass &solid_pass,
                                      const std::string &name_suffix,
                                      const std::string &texture_name,
                                      const std::string &fragment_shader,
                                      const std::string &vertex_shader)
{
    const char *fragment_shader_path = fragment_shader.c_str();
    if (fragment_shader.empty()) {
        fragment_shader_path = ":/shaders/objects/simple.frag";
    }

    const char *vertex_shader_path = vertex_shader.c_str();
    if (vertex_shader.empty()) {
        vertex_shader_path = ":/shaders/objects/object.vert";
    }

    auto &mat = resources.emplace<ffe::Material>(
                "mat/object/" + name_suffix,
                ensure_generic_object_vbo(resources),
                ensure_generic_object_ibo(resources));
    spp::EvaluationContext ctx(resources.shader_library());
    ffe::MaterialPass &pass = mat.make_pass_material(solid_pass);

    bool success = true;

    success = success && pass.shader().attach(
                resources.load_shader_checked(vertex_shader_path),
                ctx,
                GL_VERTEX_SHADER);
    success = success && pass.shader().attach(
                resources.load_shader_checked(fragment_shader_path),
                ctx,
                GL_FRAGMENT_SHADER);

    mat.declare_attribute("position", 0);
    mat.declare_attribute("tc0_in", 1);

    success = success && pass.link();

    if (!success) {
        throw std::runtime_error("failed to compile or link fallback shader material");
    }

    ffe::Texture2D &texture = load_simple_texture(
                resources,
                texture_name);

    pass.shader().bind();
    glUniform1f(pass.shader().uniform_location("texture_scale"), 1.f);
    mat.attach_texture("diffuse", &texture);
    pass.set_depth_test(false);

    return mat;
}

ffe::Material &create_intro_material(ffe::GLResourceManager &resources, ffe::RenderPass &solid_pass, const std::string &name_suffix, const std::string &texture_name)
{
    return create_simple_material(resources, solid_pass, name_suffix, texture_name,
                                  ":/shaders/intro/simple.frag",
                                  ":/shaders/intro/simple.vert");
}
