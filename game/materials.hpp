#ifndef ML_MATERIALS_H
#define ML_MATERIALS_H

#include <ffengine/gl/resource.hpp>
#include <ffengine/render/renderpass.hpp>

ffe::Texture2D &load_simple_texture(
        ffe::GLResourceManager &resources,
        const std::string &path,
        bool setup_mipmap = true);

ffe::Material &create_simple_material(
        ffe::GLResourceManager &resources,
        ffe::RenderPass &solid_pass,
        const std::string &name_suffix,
        const std::string &texture_name,
        const std::string &fragment_shader = "",
        const std::string &vertex_shader = "");

ffe::Material &create_intro_material(
        ffe::GLResourceManager &resources,
        ffe::RenderPass &solid_pass,
        const std::string &name_suffix,
        const std::string &texture_name);

ffe::Material &create_fallback_material(
        ffe::GLResourceManager &resources,
        ffe::RenderPass &solid_pass);

ffe::VBO &ensure_generic_object_vbo(ffe::GLResourceManager &resources);
ffe::IBO &ensure_generic_object_ibo(ffe::GLResourceManager &resources);

#endif
