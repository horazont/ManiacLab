/**********************************************************************
File name: TilesetEditee.hpp
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
#ifndef _ML_TILESET_EDITEE_H
#define _ML_TILESET_EDITEE_H

#include <unordered_map>

#include <sigc++/sigc++.h>

namespace sigc {
SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

#include "io/TilesetData.hpp"
#include "logic/Stamp.hpp"

class TilesetEditee;

typedef sigc::signal<void, TilesetEditee*> TilesetNotifyEvent;
typedef sigc::signal<void, TilesetEditee*, const SharedTile&> TilesetTileEvent;

class TilesetEditee
{
public:
    explicit TilesetEditee(const SharedTileset &editee);
    TilesetEditee(const TilesetEditee &ref) = delete;
    TilesetEditee& operator=(const TilesetEditee &ref) = delete;

private:
    SharedTileset _editee;
    std::unordered_map<std::string, SharedTile> _tile_map;
    TilesetNotifyEvent _changed;
    TilesetTileEvent _tile_changed;
    TilesetTileEvent _tile_created;
    TilesetTileEvent _tile_deleted;

protected:
    void changed();
    void require_unique_tile_name(const std::string &unique_name);
    void tile_deleted(const SharedTile &tile);
    void tile_changed(const SharedTile &tile);
    void tile_created(const SharedTile &tile);

public:
    const std::vector<SharedTile>& tiles() const {
        return _editee->body.tiles;
    };
    const SharedTileset &editee() const {
        return _editee;
    };

public:
    SharedTile add_tile(const SharedTile &tile);
    bool check_unique_name(const std::string &name);
    void delete_tile(const SharedTile &tile);
    SharedTile duplicate_tile(const SharedTile &src,
                              const std::string &unique_name,
                              bool rewrite_references_to_self);
    SharedTile new_tile(const std::string &unique_name);

public:
    void set_author(const std::string &value);
    void set_description(const std::string &value);
    void set_display_name(const std::string &value);
    void set_license(const std::string &value);
    void set_version(const std::string &value);

public:
    void set_tile_actor(const SharedTile &tile, bool value);
    void set_tile_blocking(const SharedTile &tile, bool value);
    void set_tile_cell_stamp(const SharedTile &tile,
                             const BoolCellStamp &stamp);
    void set_tile_destructible(const SharedTile &tile, bool value);
    void set_tile_display_name(const SharedTile &tile,
                               const std::string &value);
    void set_tile_edible(const SharedTile &tile, bool value);
    void set_tile_gravity_affected(const SharedTile &tile, bool value);
    void set_tile_movable(const SharedTile &tile, bool value);
    void set_tile_roll_radius(const SharedTile &tile,
                              float value);
    void set_tile_rollable(const SharedTile &tile, bool value);
    void set_tile_sticky(const SharedTile &tile, bool value);
    void set_tile_temp_coefficient(const SharedTile &tile,
                                   float value);

public:
    inline TilesetNotifyEvent &signal_changed() {
        return _changed;
    };

    inline TilesetTileEvent &signal_tile_changed() {
        return _tile_changed;
    };

    inline TilesetTileEvent &signal_tile_created() {
        return _tile_created;
    };

    inline TilesetTileEvent &signal_tile_deleted() {
        return _tile_deleted;
    };
};

typedef std::unique_ptr<TilesetEditee> TilesetEditeePtr;

#endif
