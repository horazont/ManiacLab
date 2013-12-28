/**********************************************************************
File name: LevelData.cpp
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
#include "LevelData.hpp"

#include <map>
#include <set>

#include <structstream/serialize.hpp>
#include <structstream/nodes.hpp>
#include <structstream/io.hpp>

#include <CEngine/IO/Log.hpp>

#include "StructstreamIntf.hpp"

using namespace StructStream;

namespace std {

template<>
struct hash<::LevelData::TileBinding>
{
    typedef ::LevelData::TileBinding argument_type;
    typedef std::size_t result_type;

private:
    hash<::SharedTileset> _tileset_hash;
    hash<::SharedTile> _tile_hash;

public:
    hash():
        _tileset_hash(),
        _tile_hash()
    {

    }

    inline result_type operator()(const argument_type &value) const
    {
        return _tileset_hash(value.first) ^ _tile_hash(value.second);
    }
};

}

/* utilities */

uint64_t map_tile(
    std::unordered_map<LevelData::TileBinding, uint64_t> &mapped_tiles,
    uint64_t &map_counter,
    const LevelData::TileBinding &binding)
{
    auto it = mapped_tiles.find(binding);
    if (it != mapped_tiles.end()) {
        return (*it).second;
    }

    uint64_t result = map_counter++;
    mapped_tiles[binding] = result;
    return result;
}


const ID SSID_LEVEL_COLLECTION_DISPLAY_NAME = 0x40;
const ID SSID_LEVEL_COLLECTION_DESCRIPTION = 0x41;
const ID SSID_LEVEL_COLLECTION_AUTHOR = 0x42;
const ID SSID_LEVEL_COLLECTION_LICENSE = 0x43;
const ID SSID_LEVEL_COLLECTION_VERSION = 0x44;
const ID SSID_LEVEL_COLLECTION_LEVEL_HEADERS = 0x45;

const ID SSID_LEVEL_HEADER = 0x4c7668;
const ID SSID_LEVEL_HEADER_DISPLAY_NAME = 0x40;
const ID SSID_LEVEL_HEADER_OFFSET = 0x41;

const ID SSID_LEVEL_BODY = 0x4d4c4c762e62;
const ID SSID_LEVEL_BODY_TILE_MAPPING = 0x40;
const ID SSID_LEVEL_BODY_PHY_INITIAL_BACKGROUND = 0x41;
const ID SSID_LEVEL_BODY_PHY_INITIAL_LAYER = 0x42;
const ID SSID_LEVEL_BODY_TILE_PLACEMENTS = 0x43;

const ID SSID_TILE_MAP_ENTRY = 0x746d;
const ID SSID_TILE_MAP_ENTRY_TILESET_NAME = 0x40;
const ID SSID_TILE_MAP_ENTRY_TILE_NAME = 0x41;
const ID SSID_TILE_MAP_ENTRY_MAPPED_ID = 0x42;

const ID SSID_PHY_INITIAL_VALUE = 0x7062;
const ID SSID_PHY_INITIAL_VALUE_AIR_PRESSURE = 0x40;
const ID SSID_PHY_INITIAL_VALUE_TEMPERATURE = 0x41;
const ID SSID_PHY_INITIAL_VALUE_FOG_DENSITY = 0x42;

const ID SSID_PHY_LAYER_VALUE = 0x706c;
const ID SSID_PHY_LAYER_VALUE_X = 0x40;
const ID SSID_PHY_LAYER_VALUE_Y = 0x41;
const ID SSID_PHY_LAYER_VALUE_ATTR = 0x42;
const ID SSID_PHY_LAYER_VALUE_VALUE = 0x43;
const ID SSID_PHY_LAYER_VALUE_ALPHA = 0x44;

const ID SSID_TILE_PLACEMENT = 0x746c;
const ID SSID_TILE_PLACEMENT_X = 0x40;
const ID SSID_TILE_PLACEMENT_Y = 0x41;
const ID SSID_TILE_PLACEMENT_MAPPED_ID = 0x42;
const ID SSID_TILE_PLACEMENT_LAYER = 0x43;

typedef struct_decl<
    Container,
    id_selector<SSID_TILE_PLACEMENT>,
    struct_members<
        member<
            UInt32Record,
            id_selector<SSID_TILE_PLACEMENT_X>,
            TilePlacement,
            uint16_t,
            &TilePlacement::x>,
        member<
            UInt32Record,
            id_selector<SSID_TILE_PLACEMENT_Y>,
            TilePlacement,
            uint16_t,
            &TilePlacement::y>,
        member<
            VarUIntRecord,
            id_selector<SSID_TILE_PLACEMENT_MAPPED_ID>,
            TilePlacement,
            VarUInt,
            &TilePlacement::mapped_id>,
        member<
            TileLayerRecord,
            id_selector<SSID_TILE_PLACEMENT_LAYER>,
            TilePlacement,
            TileLayer,
            &TilePlacement::layer>
        >
    > TilePlacementDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_PHY_LAYER_VALUE>,
    struct_members<
        member<
            UInt32Record,
            id_selector<SSID_PHY_LAYER_VALUE_X>,
            PhysicsInitialLayerOverride,
            uint16_t,
            &PhysicsInitialLayerOverride::x>,
        member<
            UInt32Record,
            id_selector<SSID_PHY_LAYER_VALUE_Y>,
            PhysicsInitialLayerOverride,
            uint16_t,
            &PhysicsInitialLayerOverride::y>,
        member<
            PhysicsInitialAttributeRecord,
            id_selector<SSID_PHY_LAYER_VALUE_ATTR>,
            PhysicsInitialLayerOverride,
            PhysicsInitialAttribute,
            &PhysicsInitialLayerOverride::attr>,
        member<
            Float32Record,
            id_selector<SSID_PHY_LAYER_VALUE_VALUE>,
            PhysicsInitialLayerValue,
            float,
            &PhysicsInitialLayerValue::value>,
        member<
            Float32Record,
            id_selector<SSID_PHY_LAYER_VALUE_ALPHA>,
            PhysicsInitialLayerValue,
            float,
            &PhysicsInitialLayerValue::alpha>
        >
    > PhysicsInitialLayerOverrideDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_PHY_INITIAL_VALUE>,
    struct_members<
        member<
            Float32Record,
            id_selector<SSID_PHY_INITIAL_VALUE_AIR_PRESSURE>,
            PhysicsInitialValue,
            float,
            &PhysicsInitialValue::air_pressure>,
        member<
            Float32Record,
            id_selector<SSID_PHY_INITIAL_VALUE_TEMPERATURE>,
            PhysicsInitialValue,
            float,
            &PhysicsInitialValue::temperature>,
        member<
            Float32Record,
            id_selector<SSID_PHY_INITIAL_VALUE_FOG_DENSITY>,
            PhysicsInitialValue,
            float,
            &PhysicsInitialValue::fog_density>
        >
    > PhysicsInitialValueDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_TILE_MAP_ENTRY>,
    struct_members<
        member<
            UTF8Record,
            id_selector<SSID_TILE_MAP_ENTRY_TILESET_NAME>,
            RawLevelTileMapEntry,
            std::string,
            &RawLevelTileMapEntry::tileset_name>,
        member<
            Raw128Record,
            id_selector<SSID_TILE_MAP_ENTRY_TILE_NAME>,
            RawLevelTileMapEntry,
            PyEngine::UUID,
            &RawLevelTileMapEntry::tile_uuid>,
        member<
            VarUIntRecord,
            id_selector<SSID_TILE_MAP_ENTRY_MAPPED_ID>,
            RawLevelTileMapEntry,
            VarUInt,
            &RawLevelTileMapEntry::mapped_id>
        >
    > RawLevelTileMapEntryDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_LEVEL_BODY>,
    struct_members<
        member_struct<
            RawLevelBodyData,
            container<
                RawLevelTileMapEntryDecl,
                id_selector<SSID_LEVEL_BODY_TILE_MAPPING>,
                std::back_insert_iterator<decltype(
                    RawLevelBodyData::tile_mapping)>
                >,
            &RawLevelBodyData::tile_mapping>,
        member_struct<
            RawLevelBodyData,
            PhysicsInitialValueDecl,
            &RawLevelBodyData::physics_initial_background>,
        member_struct<
            RawLevelBodyData,
            container<
                PhysicsInitialLayerOverrideDecl,
                id_selector<SSID_LEVEL_BODY_PHY_INITIAL_LAYER>,
                std::back_insert_iterator<decltype(
                    RawLevelBodyData::physics_initial_layer)>
                >,
            &RawLevelBodyData::physics_initial_layer>,
        member_struct<
            RawLevelBodyData,
            container<
                TilePlacementDecl,
                id_selector<SSID_LEVEL_BODY_TILE_PLACEMENTS>,
                std::back_insert_iterator<decltype(
                    RawLevelBodyData::tile_placements)>
                >,
            &RawLevelBodyData::tile_placements>
        >
    > RawLevelBodyDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_LEVEL_HEADER>,
    struct_members<
        member<
            UTF8Record,
            id_selector<SSID_LEVEL_HEADER_DISPLAY_NAME>,
            RawLevelHeaderData,
            std::string,
            &RawLevelHeaderData::display_name>,
        member<
            UInt64Record,
            id_selector<SSID_LEVEL_HEADER_OFFSET>,
            RawLevelHeaderData,
            uint64_t,
            &RawLevelHeaderData::body_stream_offset>
        >
    > RawLevelHeaderDecl;

typedef struct_decl<
    Container,
    id_selector<SSID_LEVEL_COLLECTION_HEADER>,
    struct_members<
        member<
            UTF8Record,
            id_selector<SSID_LEVEL_COLLECTION_DISPLAY_NAME>,
            RawLevelCollectionData,
            std::string,
            &RawLevelCollectionData::display_name>,
        member<
            UTF8Record,
            id_selector<SSID_LEVEL_COLLECTION_DESCRIPTION>,
            RawLevelCollectionData,
            std::string,
            &RawLevelCollectionData::description>,
        member<
            UTF8Record,
            id_selector<SSID_LEVEL_COLLECTION_AUTHOR>,
            RawLevelCollectionData,
            std::string,
            &RawLevelCollectionData::author>,
        member<
            UTF8Record,
            id_selector<SSID_LEVEL_COLLECTION_LICENSE>,
            RawLevelCollectionData,
            std::string,
            &RawLevelCollectionData::license>,
        member<
            UTF8Record,
            id_selector<SSID_LEVEL_COLLECTION_VERSION>,
            RawLevelCollectionData,
            std::string,
            &RawLevelCollectionData::version>,
        member_struct<
            RawLevelCollectionData,
            container<
                RawLevelHeaderDecl,
                id_selector<SSID_LEVEL_COLLECTION_LEVEL_HEADERS>,
                std::back_insert_iterator<
                    decltype(RawLevelCollectionData::level_headers)>
                >,
            &RawLevelCollectionData::level_headers>
        >
    > RawLevelCollectionDecl;

/* PhysicsInitialLayerOverride */

PhysicsInitialLayerOverride::PhysicsInitialLayerOverride(
        const PhysicsInitialLayerValue &value,
        const uint16_t xv,
        const uint16_t yv,
        const PhysicsInitialAttribute attrv):
    PhysicsInitialLayerValue(value),
    x(xv),
    y(yv),
    attr(attrv)
{

}

/* LevelData */

LevelData::LevelData():
    _physics_initial_background(),
    _pil_air_pressure(),
    _affector_layer(),
    _default_layer(),
    _block_map()
{
    clear();
}

LevelData::~LevelData()
{

}

void LevelData::store_physics_layer(
    RawLevelBodyData *body,
    const PhysicsLayerData &layer,
    PhysicsInitialAttribute attr)
{
    size_t i = 0;
    for (CoordInt y = 0; y < level_height*subdivision_count; y++) {
        for (CoordInt x = 0; x < level_width*subdivision_count; x++) {
            const PhysicsInitialLayerValue &value = layer[i];
            if (value.alpha <= 0) {
                i++;
                continue;
            }
            body->physics_initial_layer.push_back(
                PhysicsInitialLayerOverride(value, x, y, attr));
            i++;
        }
    }
}

LevelData::PhysicsLayerData *LevelData::get_phy_layer(
    PhysicsInitialAttribute attr)
{
    switch (attr) {
    case PHYATTR_AIR_PRESSURE:
        return &_pil_air_pressure;
    case PHYATTR_TEMPERATURE:
        return &_pil_temperature;
    case PHYATTR_FOG_DENSITY:
        return &_pil_fog_density;
    default:
        return nullptr;
    }
}

LevelData::TileLayerData *LevelData::get_tile_layer(TileLayer layer)
{
    switch (layer) {
    case TILELAYER_AFFECTOR:
        return &_affector_layer;
    case TILELAYER_DEFAULT:
        return &_default_layer;
    default:
        return nullptr;
    }
}

void LevelData::clear()
{
    _display_name = "";
    _physics_initial_background.air_pressure = 1.0;
    _physics_initial_background.temperature = 1.0;
    _physics_initial_background.fog_density = 0.0;

    clear_phy_layer(PHYATTR_AIR_PRESSURE);
    clear_phy_layer(PHYATTR_FOG_DENSITY);
    clear_phy_layer(PHYATTR_TEMPERATURE);

    clear_tile_layer(TILELAYER_AFFECTOR);
    clear_tile_layer(TILELAYER_DEFAULT);

    memset(_block_map, 0, sizeof(_block_map));
}

void LevelData::clear_phy_layer(PhysicsInitialAttribute attr)
{
    PhysicsLayerData *arr = get_phy_layer(attr);
    if (!arr) {
        return;
    }

    for (auto &value: *arr) {
        value.value = 0;
        value.alpha = 0;
    }
}

void LevelData::clear_tile_layer(TileLayer layer)
{
    TileLayerData *arr = get_tile_layer(layer);
    if (!arr) {
        return;
    }

    for (auto &value: *arr) {
        value = TileBinding(nullptr, nullptr);
    }
}

const LevelData::PhysicsLayerData &LevelData::get_phy_layer(
    PhysicsInitialAttribute attr) const
{
    switch (attr) {
    case PHYATTR_AIR_PRESSURE:
        return _pil_air_pressure;
    case PHYATTR_TEMPERATURE:
        return _pil_temperature;
    case PHYATTR_FOG_DENSITY:
        return _pil_fog_density;
    default:
        throw std::logic_error("Out-of-bounds attr: "+std::to_string(attr));
    }
}

const LevelData::TileLayerData &LevelData::get_tile_layer(
    TileLayer layer) const
{
    switch (layer) {
    case TILELAYER_AFFECTOR:
        return _affector_layer;
    case TILELAYER_DEFAULT:
        return _default_layer;
    default:
        throw std::logic_error("Out-of-bounds layer: "+std::to_string(layer));
    }
}

IOQuality LevelData::load_from_raw(
    const RawLevelHeaderData *header,
    const RawLevelBodyData *body,
    const LevelData::TileLookup &tile_lookup)
{
    IOQuality result = IOQ_PERFECT;

    clear();

    _display_name = header->display_name;

    /* create reverse mapping to speed up lookup of tiles using the
     * mapped_id from the raw data */
    std::unordered_map<uint64_t, TileBinding> tile_reverse_map;

    for (auto &map_entry: body->tile_mapping) {
        TileBinding binding = tile_lookup(
            map_entry.tileset_name,
            map_entry.tile_uuid);

        if (!binding.first)
        {
            throw std::runtime_error(
                "Failed to fulfill tileset dependency: "
                +map_entry.tileset_name);
        }
        if (!binding.second) {
            throw std::runtime_error(
                "Failed to find tile `"+map_entry.tile_uuid.to_string()+"' in ti"
                "leset `"+map_entry.tileset_name+"'.");
        }

        tile_reverse_map[map_entry.mapped_id] = binding;
    }

    /* load the physics information */
    _physics_initial_background = body->physics_initial_background;

    for (auto &ovrride: body->physics_initial_layer) {
        if (!range_check_physics_coord(ovrride.x, ovrride.y)) {
            PyEngine::log->log(PyEngine::Warning)
                << "Out of range coordinates for physics layer values: "
                << "x=" << ovrride.x << "; y=" << ovrride.y
                << PyEngine::submit;
            result = std::max(IOQ_DEGRADED, result);
            continue;
        }

        const CoordInt array_coord = physics_coord_to_array(
            ovrride.x, ovrride.y);

        PhysicsLayerData *layer = get_phy_layer(ovrride.attr);
        if (!layer) {
            PyEngine::log->log(PyEngine::Warning)
                << "Unexpected attribute layer: " << ovrride.attr
                << PyEngine::submit;
            result = std::max(IOQ_DEGRADED, result);
            continue;
        }

        PhysicsInitialLayerValue &value = (*layer)[array_coord];
        value.value = ovrride.value;
        value.alpha = ovrride.alpha;
    }

    for (auto &placement: body->tile_placements) {
        if (!range_check_game_coord(placement.x, placement.y)) {
            PyEngine::log->log(PyEngine::Warning)
                << "Out of range coordinates for game object placement:"
                << " x=" << placement.x << "; y=" << placement.y
                << PyEngine::submit;
            result = std::max(IOQ_DEGRADED, result);
            continue;
        }

        const CoordInt array_coord = game_coord_to_array(
            placement.x, placement.y);

        auto it = tile_reverse_map.find(placement.mapped_id);
        if (it == tile_reverse_map.end()) {
            PyEngine::log->log(PyEngine::Error)
                << "mapped_id " << placement.mapped_id << "not in "
                << "reverse map" << PyEngine::submit;
            result = std::max(IOQ_ERRORNOUS, result);
            continue;
        }

        const TileBinding &binding = (*it).second;

        TileLayerData *layer = get_tile_layer(placement.layer);
        if (!layer) {
            PyEngine::log->log(PyEngine::Warning)
                << "Unexpected game object layer: " << placement.layer
                << PyEngine::submit;
            result = std::max(IOQ_DEGRADED, result);
            continue;
        }

        (*layer)[array_coord] = binding;
    }

    update_block_map();
    return result;
}

void LevelData::save_to_raw(
    RawLevelHeaderData *header,
    RawLevelBodyData *body,
    const LevelData::TilesetReverseLookup &tileset_revlookup) const
{
    std::unordered_map<TileBinding, uint64_t> mapped_tiles;
    uint64_t map_counter = 0;

    header->display_name = _display_name;

    size_t i = 0;
    for (CoordInt y = 0; y < level_height; y++) {
        for (CoordInt x = 0; x < level_width; x++) {
            const TileBinding *binding = &_affector_layer[i];
            if (*binding != TileBinding(nullptr, nullptr)) {
                TilePlacement placement;
                placement.x = x;
                placement.y = y;
                placement.layer = TILELAYER_AFFECTOR;
                placement.mapped_id = map_tile(
                    mapped_tiles, map_counter, *binding);
                body->tile_placements.push_back(placement);
            }
            binding = &_default_layer[i];
            if (*binding != TileBinding(nullptr, nullptr)) {
                TilePlacement placement;
                placement.x = x;
                placement.y = y;
                placement.layer = TILELAYER_AFFECTOR;
                placement.mapped_id = map_tile(
                    mapped_tiles, map_counter, *binding);
                body->tile_placements.push_back(placement);
            }
            i++;
        }
    }

    for (auto &mapping: mapped_tiles) {
        const TileBinding &binding = mapping.first;
        RawLevelTileMapEntry entry;
        entry.tileset_name = tileset_revlookup(binding.first);
        entry.tile_uuid = binding.second->uuid;
        entry.mapped_id = mapping.second;
    }

    store_physics_layer(body, _pil_air_pressure, PHYATTR_AIR_PRESSURE);
    store_physics_layer(body, _pil_temperature, PHYATTR_TEMPERATURE);
    store_physics_layer(body, _pil_fog_density, PHYATTR_FOG_DENSITY);
}

void LevelData::update_block_map()
{
    constexpr CoordInt physics_width = level_width*subdivision_count;

    CoordInt game_cell = 0;
    for (CoordInt gy = 0; gy < level_height; gy++) {
        const CoordInt py0 = gy * subdivision_count;
        for (CoordInt gx = 0; gx < level_width; gx++) {
            const CoordInt px0 = gx * subdivision_count;
            const TileData &tile = *_default_layer[game_cell].second.get();

            for (CoordInt pyoffs = 0; pyoffs < subdivision_count; pyoffs++) {
                for (CoordInt pxoffs = 0; pxoffs < subdivision_count; pxoffs++) {
                    _block_map[(py0+pyoffs)*physics_width+pxoffs+px0] =
                        tile.stamp.get_blocking(pxoffs, pyoffs);
                }
            }

            game_cell++;
        }
    }
}

/* LevelCollection */

LevelCollection::LevelCollection():
    display_name(),
    description(),
    author(),
    license(),
    version(),
    levels()
{

}

LevelCollection::~LevelCollection()
{

}

void LevelCollection::clear()
{
    levels.clear();
    display_name = "";
    description = "";
    author = "";
    license = "";
    version = "";
}

IOQuality LevelCollection::load_from_raw(
    const RawLevelCollectionData *header,
    const PyEngine::StreamHandle &stream,
    const LevelData::TileLookup &tile_lookup)
{
    IOQuality result = IOQ_PERFECT;

    clear();

    if (header->level_headers.size() > 0) {
        uint64_t offs = header->level_headers[0].body_stream_offset;
        /* sanity check of addresses. it is required that levels are
         * stored sequentially in the file */
        if (offs != 0) {
            throw LevelIOError("First data block isn't at offset 0.");
        }

        auto it = header->level_headers.cbegin();
        ++it;
        for (; it != header->level_headers.cend(); ++it) {
            const RawLevelHeaderData &data = *it;
            if (data.body_stream_offset <= offs) {
                throw LevelIOError("Overlapping / misordered data blocks.");
            }
            offs = data.body_stream_offset;
        }
    }

    display_name = header->display_name;
    description = header->description;
    author = header->author;
    license = header->license;
    version = header->version;

    /* we assume that the stream is positioned right after the header
     * data */

    for (auto &level_header: header->level_headers) {
        std::unique_ptr<RawLevelBodyData> level_body = load_level_body(
            stream);

        std::shared_ptr<LevelData> new_level(new LevelData());
        result = std::max(result,
            new_level->load_from_raw(
                &level_header,
                level_body.get(),
                tile_lookup));

        levels.push_back(new_level);
    }

    return result;
}

void LevelCollection::save_to_stream(
    const PyEngine::StreamHandle &stream,
    const LevelData::TilesetReverseLookup &tileset_revlookup)
{
    RawLevelCollectionData header;
    header.display_name = display_name;
    header.description = description;
    header.author = author;
    header.license = license;
    header.version = version;

    std::vector<std::pair<uint8_t*, intptr_t>> level_bodies;

    uint64_t offset = 0;

    for (auto &level: levels) {
        RawLevelHeaderData level_header;
        RawLevelBodyData level_body;
        level->save_to_raw(&level_header, &level_body, tileset_revlookup);
        level_header.body_stream_offset = offset;

        header.level_headers.push_back(level_header);
        WritableMemory *io_raw = new WritableMemory();
        IOIntfHandle io(io_raw);

        serialize_to_sink<RawLevelBodyDecl>(
            level_body,
            StreamSink(new ToBitstream(io)));

        intptr_t buflen = 0;
        uint8_t *buffer = io_raw->release_buffer(buflen);
        level_bodies.push_back(std::make_pair(buffer, buflen));

        offset += buflen;
    }

    IOIntfHandle io(new PyEngineStream(stream));
    StreamSink sink(new ToBitstream(io));
    serialize_to_sink<RawLevelCollectionDecl>(header, sink);
    sink->end_of_stream();

    for (auto &item: level_bodies) {
        io->write(item.first, item.second);
        delete item.first;
    }
}
