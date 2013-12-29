/**********************************************************************
File name: LevelData.hpp
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
#ifndef _ML_LEVEL_DATA_H
#define _ML_LEVEL_DATA_H

#include <string>
#include <vector>
#include <functional>

#include <structstream/static.hpp>

#include <CEngine/Misc/UUID.hpp>

#include "logic/PhysicsConfig.hpp"

#include "Common.hpp"
#include "TilesetData.hpp"

const StructStream::ID SSID_LEVEL_COLLECTION_HEADER = 0x4d4c4c632e68;

/* The level collection format is different from the other formats
 * commonly used with structstream. Instead of having all the data in
 * one large stream, the file consists of several streams. The first
 * stream contains header information, such as level collection name,
 * authorship etc. In addition it contains byte offsets at which, from
 * the end of the header onwards, the different levels can be found.
 */

struct RawLevelHeaderData {
    PyEngine::UUID uuid;
    std::string display_name;

    uint64_t body_stream_offset;
};

struct RawLevelCollectionData {
    std::string display_name;
    std::string description;

    std::string author;
    std::string license;
    std::string version;

    std::vector<RawLevelHeaderData> level_headers;
};

struct RawLevelTileMapEntry {
    std::string tileset_name;
    PyEngine::UUID tile_uuid;
    uint64_t mapped_id;
};

struct PhysicsInitialValue {
    float air_pressure;
    float temperature;
    float fog_density;
};

struct PhysicsInitialLayerValue {
    float value;
    float alpha;
};

struct PhysicsInitialLayerOverride: public PhysicsInitialLayerValue {
    PhysicsInitialLayerOverride() = default;
    PhysicsInitialLayerOverride(
        const PhysicsInitialLayerValue &value,
        const uint16_t xv,
        const uint16_t yv,
        PhysicsInitialAttribute attrv);

    uint16_t x, y;
    PhysicsInitialAttribute attr;
};

struct TilePlacement {
    uint16_t x, y;
    uint64_t mapped_id;
    TileLayer layer;
};

struct RawLevelBodyData {
    std::vector<RawLevelTileMapEntry> tile_mapping;

    PhysicsInitialValue physics_initial_background;
    std::vector<PhysicsInitialLayerOverride> physics_initial_layer;

    std::vector<TilePlacement> tile_placements;
};

class LevelData
{
public:
    typedef std::pair<SharedTileset, SharedTile> TileBinding;
    typedef std::function<
        TileBinding(const std::string&, const PyEngine::UUID&)> TileLookup;
    typedef std::function<
        std::string(const SharedTileset&)> TilesetReverseLookup;
    static constexpr CoordInt game_cell_count = level_width*level_height;
    static constexpr CoordInt physics_cell_count = game_cell_count*subdivision_count*subdivision_count;

    typedef std::array<PhysicsInitialLayerValue, physics_cell_count> PhysicsLayerData;
    typedef std::array<TileBinding, game_cell_count> TileLayerData;

public:
    LevelData();
    ~LevelData();

private:
    PyEngine::UUID _uuid;
    std::string _display_name;

    /* pil stands for physics initial layer */
    PhysicsInitialValue _physics_initial_background;
    PhysicsLayerData _pil_air_pressure;
    PhysicsLayerData _pil_temperature;
    PhysicsLayerData _pil_fog_density;

    /* affector layer is for physics and object affectors which do not
     * collide with normal objects. such affectors can be force fields
     * or hidden helper objects */
    TileLayerData _affector_layer;
    TileLayerData _default_layer;

    /* bool map to keep track of cells which are blocked. this is mainly
     * used for visualization purposes */
    bool _block_map[physics_cell_count];

private:
    static void store_physics_layer(
        RawLevelBodyData *body,
        const PhysicsLayerData &layer,
        PhysicsInitialAttribute attr);

protected:
    PhysicsLayerData *get_phy_layer(PhysicsInitialAttribute attr);
    TileLayerData *get_tile_layer(TileLayer layer);

public:
    static bool range_check_game_coord(CoordInt x, CoordInt y);
    static bool range_check_physics_coord(CoordInt x, CoordInt y);
    static CoordInt game_coord_to_array(CoordInt x, CoordInt y);
    static CoordInt physics_coord_to_array(CoordInt x, CoordInt y);

public:
    void clear();
    void clear_phy_layer(PhysicsInitialAttribute attr);
    void clear_tile_layer(TileLayer layer);
    const PhysicsLayerData &get_phy_layer(PhysicsInitialAttribute attr) const;
    const TileLayerData &get_tile_layer(TileLayer layer) const;
    IOQuality load_from_raw(
        const RawLevelHeaderData *header,
        const RawLevelBodyData *body,
        const TileLookup &tile_lookup);
    void save_to_raw(
        RawLevelHeaderData *header,
        RawLevelBodyData *body,
        const TilesetReverseLookup &tileset_revlookup) const;
    void update_block_map();

public:
    inline const std::string &get_display_name() const
    {
        return _display_name;
    }

    inline const PyEngine::UUID &get_uuid() const
    {
        return _uuid;
    }

    inline void set_display_name(const std::string &value)
    {
        _display_name = value;
    }

    inline void set_uuid(const PyEngine::UUID &value)
    {
        _uuid = value;
    }

};

typedef std::shared_ptr<LevelData> SharedLevel;

class LevelCollection
{
public:
    LevelCollection();
    ~LevelCollection();

public:
    std::string display_name;
    std::string description;
    std::string author;
    std::string license;
    std::string version;

    std::vector<SharedLevel> levels;

public:
    void clear();
    IOQuality load_from_raw(
        const RawLevelCollectionData *header,
        const PyEngine::StreamHandle &stream,
        const LevelData::TileLookup &tile_lookup);
    void save_to_stream(
        const PyEngine::StreamHandle &stream,
        const LevelData::TilesetReverseLookup &tileset_revlookup);

};

typedef std::shared_ptr<LevelCollection> SharedLevelCollection;

std::pair<std::unique_ptr<LevelCollection>, IOQuality>
complete_level_collection_from_stream(
    const StructStream::ContainerHandle &header_root,
    const PyEngine::StreamHandle &stream,
    const LevelData::TileLookup &tile_lookup);

#endif
