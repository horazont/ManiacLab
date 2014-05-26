#include "TileMaterial.hpp"

#include <stdexcept>

#include <CEngine/GL/GeometryBufferView.hpp>

using namespace PyEngine;
using namespace PyEngine::GL;

const MaterialKey mat_player = "player";
const MaterialKey mat_safewall_standalone = "safewall0";
const MaterialKey mat_rock = "rock";

/* MetatextureObject */

void MetatextureObject::stream()
{
    index_buffer->add(vertices);
}

/* Metatexture */

Metatexture::Metatexture(
        const GeometryBufferHandle &geometry_buffer):
    geometry_buffer(geometry_buffer)
{

}

/* SimpleMetatexture */

SimpleMetatexture::SimpleMetatexture(
        const GeometryBufferHandle &geometry_buffer,
        const StreamIndexBufferHandle &index_buffer,
        const float x0,
        const float y0,
        const float x1,
        const float y1,
        const float width,
        const float height):
    Metatexture(geometry_buffer),
    index_buffer(index_buffer),
    x0(x0),
    y0(y0),
    x1(x1),
    y1(y1),
    width(width),
    height(height)
{

}

void SimpleMetatexture::set_buffer(
    MetatextureObject &tile,
    const float cx, const float cy) const
{
    GeometryBufferView view(
        geometry_buffer, tile.vertices);

    const float half_width = width / 2;
    const float half_height = height / 2;

    const std::array<float, 12> position_data({
            cx - half_width, cy - half_height, 0,
            cx - half_width, cy + half_height, 0,
            cx + half_width, cy + half_height, 0,
            cx + half_width, cy - half_height, 0});
    const std::array<float, 8> texcoord_data({
            x0, y0,
            x0, y1,
            x1, y1,
            x1, y0});

    view.getPositionView()->set(&position_data.front());
    view.getTexCoordView(0)->set(&texcoord_data.front());

    tile.cx = cx;
    tile.cy = cy;
    tile.t = 0;
}

std::unique_ptr<MetatextureObject> SimpleMetatexture::create_tile(
    const float cx, const float cy,
    const PyEngine::TimeFloat t) const
{
    auto result = std::unique_ptr<MetatextureObject>(new MetatextureObject());
    result->index_buffer = index_buffer;
    result->vertices = geometry_buffer->allocateVertices(4);

    set_buffer(*result, cx, cy);

    return result;
}

float SimpleMetatexture::get_height() const
{
    return height;
}

float SimpleMetatexture::get_width() const
{
    return width;
}

void SimpleMetatexture::update_tile(
    MetatextureObject &tile,
    const float cx, const float cy,
    const PyEngine::TimeFloat t) const
{
    if (cx != tile.cx || cy != tile.cy) {
        set_buffer(tile, cx, cy);
    }
}

/* TileMaterial */

TileMaterial::TileMaterial(
        const Metatexture *const diffuse,
        const Metatexture *const emmission):
    diffuse(diffuse),
    emmission(emmission)
{

}

/* TileMaterialManager */

TileMaterialManager::TileMaterialManager():
    _metatextures(),
    _materials()
{

}

TileMaterial *TileMaterialManager::get_material(const std::string &key) const
{
    auto it = _materials.find(key);
    if (it == _materials.end()) {
        return nullptr;
    }
    return it->second.get();
}

Metatexture *TileMaterialManager::get_metatexture(const std::string &key) const
{
    auto it = _metatextures.find(key);
    if (it == _metatextures.end()) {
        return nullptr;
    }
    return it->second.get();
}

TileMaterial *TileMaterialManager::register_material(
    const std::string &key,
    std::unique_ptr<TileMaterial> &&mat)
{
    if (_materials.find(key) != _materials.end()) {
        throw std::invalid_argument("Material key already in use: "+key);
    }

    TileMaterial *rawptr = mat.get();
    _materials[key] = std::move(mat);
    return rawptr;
}

Metatexture *TileMaterialManager::register_metatexture(
    const std::string &key,
    std::unique_ptr<Metatexture> &&tex)
{
    if (_metatextures.find(key) != _metatextures.end()) {
        throw std::invalid_argument("Material key already in use: "+key);
    }

    Metatexture *rawptr = tex.get();
    _metatextures[key] = std::move(tex);
    return rawptr;
}

TileMaterial &TileMaterialManager::require_material(
    const MaterialKey &key) const
{
    TileMaterial *obj = get_material(key);
    if (!obj) {
        throw std::invalid_argument("Invalid material: "+key);
    }

    return *obj;
}
