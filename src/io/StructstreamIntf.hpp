/**********************************************************************
File name: StructstreamIntf.hpp
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
#ifndef _ML_STRUCTSTREAM_H
#define _ML_STRUCTSTREAM_H

#include "structstream/structstream.hpp"
#include "CEngine/IO/Stream.hpp"

namespace SS = StructStream;

static const SS::ID ML_TILESET_CONT = 0x40;
static const SS::ID ML_TILESET_UNIQUE_NAME = 0x41;
static const SS::ID ML_TILESET_DISPLAY_NAME = 0x42;
static const SS::ID ML_TILESET_AUTHOR = 0x43;
static const SS::ID ML_TILESET_DESCRIPTION = 0x44;
static const SS::ID ML_TILESET_REFERENCE = 0x45;

static const SS::ID ML_TILE_CONT = 0x46;
static const SS::ID ML_TILE_UNIQUE_NAME = 0x47;
static const SS::ID ML_TILE_DISPLAY_NAME = 0x48;
static const SS::ID ML_TILE_IS_GRAVITY_AFFECTED = 0x49;
static const SS::ID ML_TILE_IS_ROLLABLE = 0x4A;
static const SS::ID ML_TILE_TEMP_COEFFICIENT = 0x4B;
static const SS::ID ML_TILE_ROLL_RADIUS = 0x4C;
static const SS::ID ML_TILE_IS_STICKY = 0x4D;
static const SS::ID ML_TILE_IS_EDIBLE = 0x4E;

static const SS::ID ML_TILE_MODEL_CONT = 0x60;
static const SS::ID ML_TILE_MODEL_IMAGE_CONT = 0x61;
static const SS::ID ML_TILE_MODEL_IMAGE_DATA = 0x62;
static const SS::ID ML_TILE_MODEL_IMAGE_DURATION = 0x63;

static const SS::ID ML_TILE_IMAGE_CONT = 0x64;
static const SS::ID ML_TILE_IMAGE_DURATION = 0x65;
static const SS::ID ML_TILE_IMAGE_IMAGE = 0x66;

static const SS::ID ML_TILESET_REFERENCE_CONT = 0x6A;
static const SS::ID ML_TILESET_REFERENCE_NAME = 0x6B;
static const SS::ID ML_TILESET_REFERENCE_EXCLUDE = 0x6C;

static const SS::RecordType RT_TILE_IMAGE = SS::RT_APPBLOB_MIN;

class TileImage;

class TileImageRecord: public SS::DataRecord
{
protected:
    explicit TileImageRecord(SS::ID id);
    TileImageRecord(const TileImageRecord &ref);
public:
    virtual ~TileImageRecord();
protected:
    SS::VarUInt _width, _height;
    void *_pixels;
public:
    virtual void read(SS::IOIntf *stream);
    virtual void write(SS::IOIntf *stream) const;

    virtual SS::RecordType record_type() const {
        return RT_TILE_IMAGE;
    };

    virtual void raw_get(void *to) const { assert(false); };
    virtual void raw_set(const void *from) { assert(false); };
    virtual intptr_t raw_size() const { assert(false); };

    virtual std::string datastr() const { assert(false); };

    virtual SS::NodeHandle copy() const;

    friend class SS::NodeHandleFactory<TileImageRecord>;
    friend class TileImage;
};

typedef std::shared_ptr<TileImageRecord> TileImageRecordHandle;

struct PyEngineStream: SS::IOIntf {
public:
    PyEngineStream(PyEngine::StreamHandle stream);
private:
    PyEngine::StreamHandle _stream_h;
    PyEngine::Stream *_stream;
public:
    virtual intptr_t read(void *buf, const intptr_t len);
    virtual intptr_t write(const void *buf, const intptr_t len);
    virtual intptr_t skip(const intptr_t len);
};

#endif
