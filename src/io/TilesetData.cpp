#include "TilesetData.hpp"

#include <CEngine/IO/Log.hpp>

#include <structstream/structstream.hpp>

#include "StructstreamIntf.hpp"

using namespace PyEngine;
using namespace StructStream;

/* ImageData */

ImageData& ImageData::operator=(const TileVisualRecord &rec)
{
    width = rec.get_width();
    height = rec.get_height();
    format = rec.get_format();
    data = std::basic_string<uint8_t>((uint8_t*)rec.get_pixels(), rec.raw_size());

    return *this;
}

/* FrameData */

void FrameData::image_data_from_record(const TileVisualRecordHandle &rec)
{
    image = *rec.get();
}

TileVisualRecordHandle FrameData::image_data_to_record(ID id) const
{
    TileVisualRecordHandle rec_h = NodeHandleFactory<TileVisualRecord>::create(id);
    TileVisualRecord &rec = *rec_h;
    rec.set(image.width, image.height, image.format, image.data.data());
    return rec_h;
}

/* structstream decls */

const ID SSID_TILESET_HEADER = 0x4d4c5473;
const ID SSID_TILESET_HEADER_DISPLAY_NAME = 0x40;
const ID SSID_TILESET_HEADER_UNIQUE_NAME = 0x41;
const ID SSID_TILESET_HEADER_DESCRIPTION = 0x42;
const ID SSID_TILESET_HEADER_AUTHOR = 0x43;
const ID SSID_TILESET_HEADER_LICENSE = 0x44;
const ID SSID_TILESET_HEADER_VERSION = 0x45;

const ID SSID_TILESET = 0x4d4c547364;
const ID SSID_TILESET_TILE = 0x40;

const ID SSID_TILE_DISPLAY_NAME = 0x40;
const ID SSID_TILE_UNIQUE_NAME = 0x41;
const ID SSID_TILE_IS_ROLLABLE = 0x42;
const ID SSID_TILE_IS_STICKY = 0x43;
const ID SSID_TILE_IS_EDIBLE = 0x44;
const ID SSID_TILE_IS_GRAVITY_AFFECTED = 0x45;
const ID SSID_TILE_ROLL_RADIUS = 0x46;
const ID SSID_TILE_TEMP_COEFFICIENT = 0x47;
const ID SSID_TILE_DEFAULT_VISUAL = 0x48;
const ID SSID_TILE_ADDITIONAL_VISUAL = 0x49;

const ID SSID_VISUAL = 0; // -- will not appear in file
const ID SSID_VISUAL_IMAGE = 0x41;

const ID SSID_FRAME = 0; // -- will not appear in file
const ID SSID_FRAME_IMAGE_DATA = 0x40;
const ID SSID_FRAME_DURATION = 0x41;

const ID SSID_VISUAL_MATCH = 0; // -- will not appear in file
const ID SSID_VISUAL_MATCH_RELEVANT = 0x40;
const ID SSID_VISUAL_MATCH_UNIQUE_NAME = 0x41;

const ID SSID_TILE_VISUAL = 0; // -- will not appear in file
const ID SSID_TILE_VISUAL_MATCH_TOP = 0x40;
const ID SSID_TILE_VISUAL_MATCH_LEFT = 0x41;
const ID SSID_TILE_VISUAL_MATCH_RIGHT = 0x42;
const ID SSID_TILE_VISUAL_MATCH_BOTTOM = 0x43;
const ID SSID_TILE_VISUAL_VISUAL = 0x44;

typedef struct_decl<
    Container,
    SSID_FRAME,
    struct_members<
        member_direct<
            TileVisualRecord,
            SSID_FRAME_IMAGE_DATA,
            FrameData,
            &FrameData::image_data_to_record,
            &FrameData::image_data_from_record>,
        member<
            Float32Record,
            SSID_FRAME_DURATION,
            FrameData,
            float,
            &FrameData::duration>
        >
    > FrameDecl;

typedef struct_decl<
    Container,
    SSID_VISUAL,
    struct_members<
        member_struct<
            VisualData,
            container<
                FrameDecl,
                std::back_insert_iterator<decltype(VisualData::frames)>
                >,
            &VisualData::frames>
        >
    > VisualDecl;

typedef struct_decl<
    Container,
    SSID_VISUAL_MATCH,
    struct_members<
        member<
            BoolRecord,
            SSID_VISUAL_MATCH_RELEVANT,
            TileVisualMatchData,
            bool,
            &TileVisualMatchData::relevant>,
        member_string<
            UTF8Record,
            SSID_VISUAL_MATCH_UNIQUE_NAME,
            TileVisualMatchData,
            &TileVisualMatchData::unique_name>
        >
    > TileVisualMatchDecl;

typedef struct_decl<
    Container,
    SSID_TILE_VISUAL,
    struct_members<
        member_struct<
            TileVisualData,
            TileVisualMatchDecl,
            &TileVisualData::top,
            SSID_TILE_VISUAL_MATCH_TOP
            >,
        member_struct<
            TileVisualData,
            TileVisualMatchDecl,
            &TileVisualData::left,
            SSID_TILE_VISUAL_MATCH_LEFT
            >,
        member_struct<
            TileVisualData,
            TileVisualMatchDecl,
            &TileVisualData::right,
            SSID_TILE_VISUAL_MATCH_RIGHT
            >,
        member_struct<
            TileVisualData,
            TileVisualMatchDecl,
            &TileVisualData::bottom,
            SSID_TILE_VISUAL_MATCH_BOTTOM
            >,
        member_struct<
            TileVisualData,
            VisualDecl,
            &TileVisualData::visual,
            SSID_TILE_VISUAL_VISUAL>
        >
    > TileVisualDecl;

typedef struct_decl<
    Container,
    SSID_TILESET_TILE,
    struct_members<
        member_string<
            UTF8Record,
            SSID_TILE_DISPLAY_NAME,
            TileData,
            &TileData::display_name>,
        member_string<
            UTF8Record,
            SSID_TILE_UNIQUE_NAME,
            TileData,
            &TileData::unique_name>,
        member<
            BoolRecord,
            SSID_TILE_IS_ROLLABLE,
            TileData,
            bool,
            &TileData::is_rollable>,
        member<
            BoolRecord,
            SSID_TILE_IS_STICKY,
            TileData,
            bool,
            &TileData::is_sticky>,
        member<
            BoolRecord,
            SSID_TILE_IS_EDIBLE,
            TileData,
            bool,
            &TileData::is_edible>,
        member<
            BoolRecord,
            SSID_TILE_IS_GRAVITY_AFFECTED,
            TileData,
            bool,
            &TileData::is_gravity_affected>,
        member<
            Float32Record,
            SSID_TILE_ROLL_RADIUS,
            TileData,
            float,
            &TileData::roll_radius>,
        member<
            Float32Record,
            SSID_TILE_TEMP_COEFFICIENT,
            TileData,
            float,
            &TileData::temp_coefficient>,
        member_struct<
            TileData,
            VisualDecl,
            &TileData::default_visual,
            SSID_TILE_DEFAULT_VISUAL>,
        member_struct<
            TileData,
            container<
                TileVisualDecl,
                std::back_insert_iterator<decltype(TileData::additional_visuals)>
                >,
            &TileData::additional_visuals,
            SSID_TILE_ADDITIONAL_VISUAL>
        >
    > TileDecl;

typedef struct_decl<
    Container,
    SSID_TILESET_HEADER,
    struct_members<
        member_string<
            UTF8Record,
            SSID_TILESET_HEADER_DISPLAY_NAME,
            TilesetHeaderData,
            &TilesetHeaderData::display_name>,
        member_string<
            UTF8Record,
            SSID_TILESET_HEADER_UNIQUE_NAME,
            TilesetHeaderData,
            &TilesetHeaderData::unique_name>,
        member_string<
            UTF8Record,
            SSID_TILESET_HEADER_DESCRIPTION,
            TilesetHeaderData,
            &TilesetHeaderData::description>,
        member_string<
            UTF8Record,
            SSID_TILESET_HEADER_LICENSE,
            TilesetHeaderData,
            &TilesetHeaderData::license>,
        member_string<
            UTF8Record,
            SSID_TILESET_HEADER_VERSION,
            TilesetHeaderData,
            &TilesetHeaderData::version>
        >
    > TilesetHeaderDecl;

typedef struct_decl<
    Container,
    SSID_TILESET,
    struct_members<
        member_struct<
            TilesetBodyData,
            container<
                TileDecl,
                std::back_insert_iterator<decltype(TilesetBodyData::tiles)>
                >,
            &TilesetBodyData::tiles>
        >
    > TilesetBodyDecl;

/* free functions */

std::unique_ptr<TilesetData> load_tileset_from_stream(const StreamHandle &stream)
{
    std::unique_ptr<TilesetData> result(new TilesetData());
    IOIntfHandle io(new PyEngineStream(stream));

    StreamSink sink(new SinkChain({
        deserialize<only<TilesetHeaderDecl, true, true>>(result->header),
        deserialize<only<TilesetBodyDecl, true, true>>(result->body)
    }));

    try {
        FromBitstream(
            io,
            maniac_lab_registry,
            sink).read_all();
    } catch (const RecordNotFound &err) {
        return nullptr;
    } catch (const std::runtime_error &err) {
        PyEngine::log->log(Error)
            << "While loading tileset: " << err.what() << std::endl;
        return nullptr;
    }

    return result;
}

std::unique_ptr<TilesetHeaderData> load_tileset_header_from_stream(const StreamHandle &stream)
{
    std::unique_ptr<TilesetHeaderData> result(new TilesetHeaderData());
    IOIntfHandle io(new PyEngineStream(stream));

    try {
        FromBitstream(
            io,
            maniac_lab_registry,
            deserialize<only<TilesetHeaderDecl, true, true>>(*result.get())).read_all();
    } catch (const RecordNotFound &err) {
        return nullptr;
    } catch (const std::runtime_error &err) {
        PyEngine::log->log(Error)
            << "While loading tileset: " << err.what() << std::endl;
        return nullptr;
    }

    return result;
}

void save_tileset_to_stream(
    const TilesetData &tileset,
    const PyEngine::StreamHandle &stream)
{
    IOIntfHandle io(new PyEngineStream(stream));
    StreamSink sink(new ToBitstream(io));
    serialize_to_sink<TilesetHeaderDecl>(tileset.header, sink);
    serialize_to_sink<TilesetBodyDecl>(tileset.body, sink);
    sink->end_of_stream();
}