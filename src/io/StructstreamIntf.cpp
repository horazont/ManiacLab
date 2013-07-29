/**********************************************************************
File name: StructstreamIntf.cpp
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
#include "StructstreamIntf.hpp"

#include <unistd.h>

#include <CEngine/IO/Log.hpp>

using namespace StructStream;
using namespace PyEngine;

TileVisualRecord::TileVisualRecord(ID id):
    DataRecord(id),
    _width(0),
    _height(0),
    _format(TVF_LUMINANCE),
    _pixels(0)
{

}

TileVisualRecord::TileVisualRecord(const TileVisualRecord &ref):
    DataRecord(ref._id),
    _width(ref._width),
    _height(ref._height),
    _format(ref._format),
    _pixels((uint8_t*)malloc(ref.raw_size()))
{
    memcpy(_pixels, ref._pixels, raw_size());
}

TileVisualRecord::~TileVisualRecord()
{
    free(_pixels);
    _pixels = nullptr;
}

void TileVisualRecord::read(IOIntf *stream)
{
    VarUInt new_width = Utils::read_varuint(stream);
    VarUInt new_height = Utils::read_varuint(stream);
    TileVisualFormat new_format = (TileVisualFormat)Utils::read_varuint(stream);

    if (get_pixel_size(new_format) == 0) {
        PyEngine::log->getChannel("io")->log(Error)
            << "Unknown pixel format encountered: " << new_format
            << submit;
        throw std::runtime_error("Unknown pixel format encountered.");
    }

    intptr_t old_size = raw_size();

    _width = new_width;
    _height = new_height;
    _format = new_format;

    static const intptr_t chunk_size = 1<<20;
    const intptr_t new_size = raw_size();

    intptr_t to_read = raw_size();
    if (to_read <= old_size) {
        sread(stream, _pixels, to_read);
        _pixels = (uint8_t*)realloc(_pixels, to_read);
        assert(_pixels);
        return;
    }

    intptr_t current_size = old_size;
    intptr_t read_offs = 0;
    while (to_read > 0) {
        const intptr_t curr_step = std::min(chunk_size, to_read);
        const intptr_t new_offs = read_offs + curr_step;
        if (new_offs > current_size) {
            void *new_pixels = realloc(_pixels, new_offs);
            if (!new_pixels) {
                new_pixels = realloc(_pixels, 1);
                assert(new_pixels); // allocation of 1 must be possible...
                *((uint8_t*)_pixels) = 0;
                _width = 1;
                _height = 1;
                _format = TVF_LUMINANCE;
                PyEngine::log->log(Error)
                    << "Memory allocation failure while reading visual "
                    << "of size " << new_size << "." << submit;
                return;
            }
            _pixels = (uint8_t*)new_pixels;
            current_size = new_offs;
        }

        sread(stream, &((uint8_t*)_pixels)[read_offs], curr_step);
        to_read -= curr_step;
    }
}

void TileVisualRecord::write(IOIntf *stream) const
{
    write_header(stream);

    Utils::write_varuint(stream, _width);
    Utils::write_varuint(stream, _height);

    const VarUInt size = _width*_height*4;
    swrite(stream, _pixels, size);
}

intptr_t TileVisualRecord::raw_size() const
{
    const intptr_t pixel_size = get_pixel_size(_format);
    return pixel_size * _width * _height;
}

NodeHandle TileVisualRecord::copy() const
{
    return NodeHandleFactory<TileVisualRecord>::copy(*this);
}

void TileVisualRecord::set(
        VarUInt width, VarUInt height,
        TileVisualFormat format,
        uint8_t const*pixels)
{
    const intptr_t new_size = width*height*get_pixel_size(format);
    uint8_t *new_pixels = (uint8_t*)realloc(_pixels, new_size);
    if (!new_pixels) {
        throw std::runtime_error("Out of memory: cannot set visual record contents.");
    }
    _width = width;
    _height = height;
    _format = format;
    _pixels = new_pixels;
    memcpy(_pixels, pixels, new_size);
}

/* PyEngineStream */

PyEngineStream::PyEngineStream(StreamHandle stream):
    _stream_h(stream),
    _stream(stream.get())
{

}

intptr_t PyEngineStream::read(void *buf, const intptr_t len)
{
    return _stream->read(buf, len);
}

intptr_t PyEngineStream::write(const void *buf, const intptr_t len)
{
    return _stream->write(buf, len);
}

intptr_t PyEngineStream::skip(const intptr_t len)
{
    if (_stream->isSeekable()) {
        intptr_t old_pos = _stream->tell();
        intptr_t new_pos = _stream->seek(SEEK_CUR, len);
        return new_pos - old_pos;
    } else {
        return this->IOIntf::skip(len);
    }
}

/* free functions */

intptr_t get_pixel_size(TileVisualFormat format)
{
    switch (format) {
    case TVF_LUMINANCE:
        return 1;
    case TVF_LUMINANCE_ALPHA:
        return 2;
    case TVF_RGBA:
        return 4;
    default:
        return 0;
    };
}

RegistryHandle maniac_lab_registry = RegistryHandle(new Registry({
    std::pair<RecordType, NodeConstructor>(RT_TILE_VISUAL, &NodeHandleFactory<TileVisualRecord>::create)
    }));
