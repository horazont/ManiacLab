#include "TileMaterial.hpp"

#include <CEngine/GL/GeometryBufferView.hpp>

using namespace PyEngine;
using namespace PyEngine::GL;

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
        const GLuint textureid,
        const float x0,
        const float y0,
        const float x1,
        const float y1,
        const float width,
        const float height):
    Metatexture(geometry_buffer),
    index_buffer(index_buffer),
    textureid(textureid),
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
    const float cx, const float cy)
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
    const PyEngine::TimeFloat t)
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
    const PyEngine::TimeFloat t)
{
    if (cx != tile.cx || cy != tile.cy) {
        set_buffer(tile, cx, cy);
    }
}

/* TileMaterialManager */

TileMaterialManager::TileMaterialManager():
    _metatextures(),
    _materials()
{

}

TileMaterial *TileMaterialManager::get_material(const std::string &key) const
{
    return _materials[key].get();
}

Metatexture *TileMaterialManager::get_metatexture(const std::string &key) const
{
    return _metatextures[key].get();
}

void TileMaterialManager::register_material(
    const std::string &key,
    std::unique_ptr<TileMaterial> &&mat)
{
    if (_materials.find(key) != _materials.end()) {
        throw std::lookup_error("Material key already in use: "+key);
    }
    _materials[key] = std::move(mat);
}

void TileMaterialManager::register_metatexture(
    const std::string &key,
    std::unique_ptr<Metatexture> &&tex)
{
    if (_metatextures.find(key) != _metatextures.end()) {
        throw std::lookup_error("Material key already in use: "+key);
    }
    _metatextures[key] = std::move(tex);
}
