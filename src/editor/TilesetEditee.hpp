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

#include "io/TilesetData.hpp"
#include "logic/Stamp.hpp"

class TilesetEditee;

typedef sigc::signal<void, TilesetEditee*> TilesetNotifyEvent;
typedef sigc::signal<void, TilesetEditee*, const SharedTile&> TilesetTileEvent;

class TilesetEditee
{
public:
    TilesetEditee(const SharedTileset &editee, const std::string &name);
    TilesetEditee(const TilesetEditee &ref) = delete;
    TilesetEditee& operator=(const TilesetEditee &ref) = delete;

private:
    std::string _name;
    SharedTileset _editee;
    std::unordered_map<PyEngine::UUID, SharedTile> _tile_map;
    TilesetNotifyEvent _changed;
    TilesetTileEvent _tile_changed;
    TilesetTileEvent _tile_created;
    TilesetTileEvent _tile_deleted;

protected:
    void changed();
    void require_valid_tile_uuid(const PyEngine::UUID &uuid);
    void tile_deleted(const SharedTile &tile);
    void tile_created(const SharedTile &tile);

public:
    const std::vector<SharedTile>& tiles() const {
        return _editee->body.tiles;
    };

    const SharedTileset &editee() const {
        return _editee;
    };

    const std::string &get_name() const {
        return _name;
    };

public:
    SharedTile add_tile(const SharedTile &tile);
    bool check_uuid(const PyEngine::UUID &name);
    void delete_tile(const SharedTile &tile);
    SharedTile duplicate_tile(const SharedTile &src,
                              const PyEngine::UUID &uuid,
                              bool rewrite_references_to_self);
    SharedTile find_tile(const PyEngine::UUID &uuid) const;
    SharedTile new_tile(const PyEngine::UUID &uuid);

public:
    void set_name(const std::string &name);

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
                             const CellStamp &stamp);
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
    void tile_changed(const SharedTile &tile);

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
