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

#include "logic/Tileset.hpp"

#include <unistd.h>

using namespace StructStream;
using namespace PyEngine;

TileImageRecord::TileImageRecord(ID id):
    DataRecord(id),
    _width(0),
    _height(0),
    _pixels(0)
{

}

TileImageRecord::TileImageRecord(const TileImageRecord &ref):
    DataRecord(ref._id),
    _width(ref._width),
    _height(ref._height),
    _pixels(malloc(_width*_height*TileImage::BytesPerPixel))
{
    memcpy(_pixels, ref._pixels, _width*_height*TileImage::BytesPerPixel);
}

TileImageRecord::~TileImageRecord()
{
    free(_pixels);
    _pixels = nullptr;
}

void TileImageRecord::read(IOIntf *stream)
{
    _width = Utils::read_varuint(stream);
    _height = Utils::read_varuint(stream);

    // TODO: insert limits check here

    const VarUInt size = _width*_height*TileImage::BytesPerPixel;

    _pixels = realloc(_pixels, size);
    sread(stream, _pixels, size);
}

void TileImageRecord::write(IOIntf *stream) const
{
    write_header(stream);

    Utils::write_varuint(stream, _width);
    Utils::write_varuint(stream, _height);

    const VarUInt size = _width*_height*TileImage::BytesPerPixel;
    swrite(stream, _pixels, size);
}

NodeHandle TileImageRecord::copy() const
{
    return NodeHandleFactory<TileImageRecord>::copy(*this);
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
