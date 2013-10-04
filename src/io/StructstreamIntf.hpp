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

#include <CEngine/IO/Stream.hpp>

#include <structstream/structstream.hpp>

#include "Common.hpp"
#include "logic/Stamp.hpp"

namespace SS = StructStream;

static const SS::RecordType RT_TILE_VISUAL = SS::RT_APP_NOSIZE_MIN;
static const SS::RecordType RT_CELL_TYPE = RT_TILE_VISUAL+1;
static const SS::RecordType RT_SINK_SOURCE_TYPE = RT_CELL_TYPE+1;
static const SS::RecordType RT_PHYSICS_INITIAL_ATTRIBUTE = RT_SINK_SOURCE_TYPE+1;
static const SS::RecordType RT_TILE_LAYER = RT_PHYSICS_INITIAL_ATTRIBUTE+1;

class TileVisualRecord: public SS::DataRecord
{
protected:
    explicit TileVisualRecord(SS::ID id);
    TileVisualRecord(const TileVisualRecord &ref);

public:
    virtual ~TileVisualRecord();

protected:
    SS::VarUInt _width, _height;
    TileVisualFormat _format;
    uint8_t *_pixels;

public:
    virtual void read(SS::IOIntf *stream);
    virtual void write(SS::IOIntf *stream) const;

    virtual SS::RecordType record_type() const {
        return RT_TILE_VISUAL;
    };

    virtual void raw_get(void *to) const { assert(false); };
    virtual void raw_set(const void *from) { assert(false); };
    virtual intptr_t raw_size() const;

    virtual std::string datastr() const { assert(false); };

    virtual SS::NodeHandle copy() const;

    friend class SS::NodeHandleFactory<TileVisualRecord>;

    inline SS::VarUInt get_width() const {
        return _width;
    };

    inline SS::VarUInt get_height() const {
        return _height;
    };

    inline uint8_t const* get_pixels() const {
        return _pixels;
    };

    inline TileVisualFormat get_format() const {
        return _format;
    };

    void set(SS::VarUInt width, SS::VarUInt height,
             TileVisualFormat format,
             uint8_t const*pixels);
};

typedef SS::EnumRecordTpl<
    CellType,
    RT_CELL_TYPE,
    SS::VarUIntRecord> CellTypeRecord;

typedef SS::EnumRecordTpl<
    SinkSourceType,
    RT_SINK_SOURCE_TYPE,
    SS::VarUIntRecord> SinkSourceTypeRecord;

typedef SS::EnumRecordTpl<
    PhysicsInitialAttribute,
    RT_PHYSICS_INITIAL_ATTRIBUTE,
    SS::VarUIntRecord> PhysicsInitialAttributeRecord;

typedef SS::EnumRecordTpl<
    TileLayer,
    RT_TILE_LAYER,
    SS::VarIntRecord> TileLayerRecord;

typedef std::shared_ptr<TileVisualRecord> TileVisualRecordHandle;

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

intptr_t get_pixel_size(TileVisualFormat format);

extern SS::RegistryHandle maniac_lab_registry;

#endif
