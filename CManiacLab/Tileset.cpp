/**********************************************************************
File name: Tileset.cpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "Tileset.hpp"

using namespace PyEngine;
using namespace StructStream;

/* TileImage */

TileImage::TileImage():
    _width(0),
    _height(0),
    _pixels(nullptr),
    _duration(0)
{

}

TileImage::TileImage(const TileImage &ref):
    _width(ref._width),
    _height(ref._height),
    _pixels(malloc(ref.size())),
    _duration(0)
{
    memcpy(_pixels, ref._pixels, size());
}

TileImage& TileImage::operator= (const TileImage &ref)
{
    _pixels = realloc(_pixels, ref.size());
    assert(_pixels);

    _width = ref._width;
    _height = ref._height;
    memcpy(_pixels, ref._pixels, size());
    _duration = ref._duration;

    return *this;
}

TileImage::~TileImage()
{
    clear();
}

void TileImage::clear()
{
    if (_pixels) {
        free(_pixels);
        _pixels = nullptr;
    }
    _width = 0;
    _height = 0;
    _duration = 0;
}

TileImageRecordHandle TileImage::get_image_record(SS::ID id) const
{
    TileImageRecord *rec = new TileImageRecord(id);
    rec->_width = _width;
    rec->_height = _height;
    rec->_pixels = malloc(size());
    memcpy(rec->_pixels, _pixels, size());

    return TileImageRecordHandle(rec);
}

void TileImage::set_image_record(TileImageRecordHandle rec)
{
    _width = rec->_width;
    _height = rec->_height;
    _pixels = realloc(_pixels, size());
    memcpy(_pixels, rec->_pixels, size());
}

void TileImage::texImage2D(const GLenum target, const GLint level,
                           const GLint internal_format) const
{
    glTexImage2D(target, level, internal_format, _width, _height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels);
}

void TileImage::texSubImage2D(const GLenum target, const GLint level,
                              const GLint internal_format,
                               const GLint x, const GLint y) const
{
    glTexSubImage2D(target, level, x, y, _width, _height,
                    GL_RGBA, GL_UNSIGNED_BYTE, _pixels);
}

/* TileModel */

TileModel::TileModel():
    images()
{

}

TileModel::TileModel(const TileModel &ref):
    images(ref.images)
{

}

TileModel::~TileModel()
{

}

/* Tile */

Tile::Tile():
    unique_name(),
    display_name("Unnamed tile"),
    is_gravity_affected(false),
    is_rollable(false),
    is_sticky(false),
    is_edible(false),
    temp_coefficient(1),
    roll_radius(0),
    ca_stamp(),
    model()
{

}

Tile::Tile(const Tile &ref):
    unique_name(),
    display_name(ref.display_name),
    is_gravity_affected(ref.is_gravity_affected),
    is_rollable(ref.is_rollable),
    is_sticky(ref.is_sticky),
    is_edible(ref.is_edible),
    temp_coefficient(ref.temp_coefficient),
    roll_radius(ref.roll_radius),
    ca_stamp(),
    model(ref.model)
{

}

Tile::~Tile()
{

}

/* Tileset */

Tileset::Tileset():
    _unique_name(),
    _display_name("Unnamed tileset"),
    _author(),
    _description(),
    _referenced_tilesets(),
    _tiles()
{

}

Tileset::~Tileset()
{

}

void Tileset::add_referenced_tileset(TilesetReference &&tileset_reference)
{
    _referenced_tilesets.push_back(std::move(tileset_reference));
}

void Tileset::add_tile(TileHandle &&tile)
{
    _tiles.push_back(std::move(tile));
}

void Tileset::save_to_stream(StreamHandle stream) const
{
    StreamSink sink(new ToBitstream(IOIntfHandle(new PyEngineStream(stream))));
    serialize_to_sink<tileset_decl>(*this, sink);
    sink->end_of_stream();
}

Tileset* Tileset::load_from_stream(StreamHandle stream)
{
    Tileset *tileset = new Tileset();
    FromBitstream(
        IOIntfHandle(new PyEngineStream(stream)),
        RegistryHandle(new Registry()),
        deserialize<only<tileset_decl>>(*tileset)).read_all();
    return tileset;
}

/* TilesetInfo */

TilesetInfo *TilesetInfo::read_from_stream(StreamHandle stream)
{
    TilesetInfo *result = new TilesetInfo();
    try {
        FromBitstream(IOIntfHandle(new PyEngineStream(stream)),
                      RegistryHandle(new Registry()),
                      deserialize<only<tileset_info_decl>>(*result)).read_all();
    } catch (...) {
        delete result;
        throw;
    }

    if (result->unique_name == "") {
        delete result;
        return nullptr;
    }

    return result;
}
