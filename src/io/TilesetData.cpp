/**********************************************************************
File name: TilesetData.cpp
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

/* TileData */

TileData::TileData():
    unique_name(),
    display_name(),
    is_blocking(false),
    is_destructible(false),
    is_edible(false),
    is_gravity_affected(false),
    is_rollable(false),
    is_sticky(false),
    roll_radius(1),
    temp_coefficient(1),
    default_visual(),
    additional_visuals()
{

}

/* structstream decls */

const ID SSID_TILESET_HEADER_DISPLAY_NAME = 0x40;
const ID SSID_TILESET_HEADER_UNIQUE_NAME = 0x41;
const ID SSID_TILESET_HEADER_DESCRIPTION = 0x42;
const ID SSID_TILESET_HEADER_AUTHOR = 0x43;
const ID SSID_TILESET_HEADER_LICENSE = 0x44;
const ID SSID_TILESET_HEADER_VERSION = 0x45;

const ID SSID_TILESET = 0x4d4c547364;
const ID SSID_TILESET_TILES = 0x40;

const ID SSID_TILE = 0x746c;
const ID SSID_TILE_DISPLAY_NAME = 0x40;
const ID SSID_TILE_UNIQUE_NAME = 0x41;
const ID SSID_TILE_IS_ROLLABLE = 0x42;
const ID SSID_TILE_IS_STICKY = 0x43;
const ID SSID_TILE_IS_EDIBLE = 0x44;
const ID SSID_TILE_IS_GRAVITY_AFFECTED = 0x45;
const ID SSID_TILE_ROLL_RADIUS = 0x46;
const ID SSID_TILE_TEMP_COEFFICIENT = 0x47;
const ID SSID_TILE_DEFAULT_VISUAL = 0x48;
const ID SSID_TILE_ADDITIONAL_VISUALS = 0x49;
const ID SSID_TILE_IS_BLOCKING = 0x4A;
const ID SSID_TILE_IS_DESTRUCTIBLE = 0x4B;
const ID SSID_TILE_IS_ACTOR = 0x4C;
const ID SSID_TILE_IS_MOVABLE = 0x4D;

const ID SSID_VISUAL = 0x7673;
const ID SSID_VISUAL_FRAMES = 0x40;

const ID SSID_FRAME = 0x6672;
const ID SSID_FRAME_IMAGE_DATA = 0x40;
const ID SSID_FRAME_DURATION = 0x41;

const ID SSID_VISUAL_MATCH = 0x766d;
const ID SSID_VISUAL_MATCH_RELEVANT = 0x40;
const ID SSID_VISUAL_MATCH_UNIQUE_NAME = 0x41;

const ID SSID_TILE_VISUAL = 0x5476;
const ID SSID_TILE_VISUAL_MATCH_TOP = 0x40;
const ID SSID_TILE_VISUAL_MATCH_LEFT = 0x41;
const ID SSID_TILE_VISUAL_MATCH_RIGHT = 0x42;
const ID SSID_TILE_VISUAL_MATCH_BOTTOM = 0x43;
const ID SSID_TILE_VISUAL_VISUAL = 0x44;

const ID SSID_CELL_STAMP = 0x6373;
const ID SSID_CELL_STAMP_TEMPLATES = 0x40;

const ID SSID_CELL_TEMPLATE = 0x6374;
const ID SSID_CELL_TEMPLATE_TYPE = 0x40;
const ID SSID_CELL_TEMPLATE_AMPLITUDE = 0x41;
const ID SSID_CELL_TEMPLATE_MATERIAL = 0x42;
const ID SSID_CELL_TEMPLATE_FLOW_X = 0x43;
const ID SSID_CELL_TEMPLATE_FLOW_Y = 0x44;

typedef struct_decl<
    Container,
    id_selector<SSID_CELL_TEMPLATE>,
    struct_members<
        member<
            CellTypeRecord,
            id_selector<SSID_CELL_TEMPLATE_TYPE>,
            CellTemplate,
            CellType,
            &CellTemplate::type>,
        member<
            SinkSourceTypeRecord,
            id_selector<SSID_CELL_TEMPLATE_MATERIAL>,
            CellTemplate,
            SinkSourceType,
            &CellTemplate::sink_what>,
        member<
            Float32Record,
            id_selector<SSID_CELL_TEMPLATE_AMPLITUDE>,
            CellTemplate,
            float,
            &CellTemplate::amplitude>,
        member<
            Float32Record,
            id_selector<SSID_CELL_TEMPLATE_FLOW_X>,
            CellTemplate,
            float,
            &CellTemplate::flow_west>,
        member<
            Float32Record,
            id_selector<SSID_CELL_TEMPLATE_FLOW_Y>,
            CellTemplate,
            float,
            &CellTemplate::flow_north>
        >
    > CellTemplateDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_CELL_STAMP>,
    struct_members<
        member_struct<
            CellStamp,
            static_array<
                CellTemplateDecl,
                id_selector<SSID_CELL_STAMP_TEMPLATES>,
                cell_stamp_length>,
            &CellStamp::data>
        >
    > CellStampDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_FRAME>,
    struct_members<
        member_direct<
            FrameData,
            id_selector<SSID_FRAME_IMAGE_DATA>,
            TileVisualRecord,
            &FrameData::image_data_to_record,
            &FrameData::image_data_from_record>,
        member<
            Float32Record,
            id_selector<SSID_FRAME_DURATION>,
            FrameData,
            float,
            &FrameData::duration>
        >
    > FrameDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_VISUAL>,
    struct_members<
        member_struct<
            VisualData,
            container<
                FrameDecl,
                id_selector<SSID_VISUAL_FRAMES>,
                std::back_insert_iterator<decltype(VisualData::frames)>
                >,
            &VisualData::frames>
        >
    > VisualDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_VISUAL_MATCH>,
    struct_members<
        member<
            BoolRecord,
            id_selector<SSID_VISUAL_MATCH_RELEVANT>,
            TileVisualMatchData,
            bool,
            &TileVisualMatchData::relevant>,
        member<
            UTF8Record,
            id_selector<SSID_VISUAL_MATCH_UNIQUE_NAME>,
            TileVisualMatchData,
            std::string,
            &TileVisualMatchData::unique_name>
        >
    > TileVisualMatchDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_TILE_VISUAL>,
    struct_members<
        member_struct_wrap<
            TileVisualData,
            TileVisualMatchDecl,
            id_selector<SSID_TILE_VISUAL_MATCH_TOP>,
            &TileVisualData::top
            >,
        member_struct_wrap<
            TileVisualData,
            TileVisualMatchDecl,
            id_selector<SSID_TILE_VISUAL_MATCH_LEFT>,
            &TileVisualData::left
            >,
        member_struct_wrap<
            TileVisualData,
            TileVisualMatchDecl,
            id_selector<SSID_TILE_VISUAL_MATCH_RIGHT>,
            &TileVisualData::right
            >,
        member_struct_wrap<
            TileVisualData,
            TileVisualMatchDecl,
            id_selector<SSID_TILE_VISUAL_MATCH_BOTTOM>,
            &TileVisualData::bottom
            >,
        member_struct_wrap<
            TileVisualData,
            VisualDecl,
            id_selector<SSID_TILE_VISUAL_VISUAL>,
            &TileVisualData::visual>
        >
    > TileVisualDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_TILE>,
    struct_members<
        member<
            UTF8Record,
            id_selector<SSID_TILE_DISPLAY_NAME>,
            TileData,
            std::string,
            &TileData::display_name>,
        member<
            UTF8Record,
            id_selector<SSID_TILE_UNIQUE_NAME>,
            TileData,
            std::string,
            &TileData::unique_name>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_ACTOR>,
            TileData,
            bool,
            &TileData::is_actor>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_BLOCKING>,
            TileData,
            bool,
            &TileData::is_blocking>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_DESTRUCTIBLE>,
            TileData,
            bool,
            &TileData::is_destructible>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_EDIBLE>,
            TileData,
            bool,
            &TileData::is_edible>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_GRAVITY_AFFECTED>,
            TileData,
            bool,
            &TileData::is_gravity_affected>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_MOVABLE>,
            TileData,
            bool,
            &TileData::is_movable>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_ROLLABLE>,
            TileData,
            bool,
            &TileData::is_rollable>,
        member<
            BoolRecord,
            id_selector<SSID_TILE_IS_STICKY>,
            TileData,
            bool,
            &TileData::is_sticky>,
        member<
            Float32Record,
            id_selector<SSID_TILE_ROLL_RADIUS>,
            TileData,
            float,
            &TileData::roll_radius>,
        member<
            Float32Record,
            id_selector<SSID_TILE_TEMP_COEFFICIENT>,
            TileData,
            float,
            &TileData::temp_coefficient>,
        member_struct<
            TileData,
            VisualDecl,
            &TileData::default_visual>,
        member_struct<
            TileData,
            container<
                TileVisualDecl,
                id_selector<SSID_TILE_ADDITIONAL_VISUALS>,
                std::back_insert_iterator<decltype(TileData::additional_visuals)>
                >,
            &TileData::additional_visuals>,
        member_struct<
            TileData,
            CellStampDecl,
            &TileData::stamp>
        >
    > TileDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_TILESET_HEADER>,
    struct_members<
        member<
            UTF8Record,
            id_selector<SSID_TILESET_HEADER_DISPLAY_NAME>,
            TilesetHeaderData,
            std::string,
            &TilesetHeaderData::display_name>,
        member<
            UTF8Record,
            id_selector<SSID_TILESET_HEADER_DESCRIPTION>,
            TilesetHeaderData,
            std::string,
            &TilesetHeaderData::description>,
        member<
            UTF8Record,
            id_selector<SSID_TILESET_HEADER_AUTHOR>,
            TilesetHeaderData,
            std::string,
            &TilesetHeaderData::author>,
        member<
            UTF8Record,
            id_selector<SSID_TILESET_HEADER_LICENSE>,
            TilesetHeaderData,
            std::string,
            &TilesetHeaderData::license>,
        member<
            UTF8Record,
            id_selector<SSID_TILESET_HEADER_VERSION>,
            TilesetHeaderData,
            std::string,
            &TilesetHeaderData::version>
        >
    > TilesetHeaderDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_TILESET>,
    struct_members<
        member_struct<
            TilesetBodyData,
            container<
                TileDecl,
                id_selector<SSID_TILESET_TILES>,
                std::back_insert_iterator<decltype(TilesetBodyData::tiles)>
                >,
            &TilesetBodyData::tiles>
        >
    > TilesetBodyDecl;

/* free functions */

std::pair<StreamSink, std::unique_ptr<TilesetData>>
    create_tileset_sink()
{
    std::unique_ptr<TilesetData> result(new TilesetData());
    StreamSink sink(new SinkChain({
        deserialize<only<TilesetHeaderDecl, true, true>>(result->header),
        deserialize<only<TilesetBodyDecl, true, true>>(result->body)
    }));

    return std::make_pair(sink, std::move(result));
}

std::unique_ptr<TilesetData> load_tileset_from_tree(const ContainerHandle &root)
{
    tree_debug(root, std::cout);

    std::unique_ptr<TilesetData> result;
    StreamSink sink;
    std::tie(sink, result) = create_tileset_sink();

    try {
        FromTree(sink, root);
    } catch (const RecordNotFound &err) {
        PyEngine::log->log(Error)
            << "Given tree is not a tileset: " << err.what() << submit;
        return nullptr;
    }

    return result;
}

std::unique_ptr<TilesetData> load_tileset_from_stream(const StreamHandle &stream)
{
    IOIntfHandle io(new PyEngineStream(stream));

    std::unique_ptr<TilesetData> result;
    StreamSink sink;
    std::tie(sink, result) = create_tileset_sink();

    try {
        FromBitstream(
            io,
            maniac_lab_registry,
            sink).read_all();
    } catch (const RecordNotFound &err) {
        PyEngine::log->log(Error)
            << "Given stream is not a tileset: " << err.what() << submit;
        return nullptr;
    } catch (const std::runtime_error &err) {
        PyEngine::log->log(Error)
            << "While loading tileset: " << err.what() << submit;
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
            << "While loading tileset: " << err.what() << submit;
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
