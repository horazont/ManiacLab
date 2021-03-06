/**********************************************************************
File name: TilesetEditee.cpp
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
#include "TilesetEditee.hpp"

/* TilesetEditee */

TilesetEditee::TilesetEditee(
        const SharedTileset &editee,
        const std::string &name):
    _name(name),
    _editee(editee),
    _changed(),
    _tile_changed(),
    _tile_created(),
    _tile_deleted()
{
    for (auto& tile: tiles()) {
        _tile_map[tile->uuid] = tile;
    }
}

void TilesetEditee::changed()
{
    _changed(this);
}

void TilesetEditee::require_valid_tile_uuid(
    const PyEngine::UUID &uuid)
{
    if (!check_uuid(uuid)) {
        throw std::invalid_argument(
            "Tile UUID conflict (or nil UUID): "+uuid.to_string());
    }
}

void TilesetEditee::tile_created(const SharedTile &tile)
{
    _tile_created(this, tile);
    changed();
}

void TilesetEditee::tile_deleted(const SharedTile &tile)
{
    _tile_deleted(this, tile);
    changed();
}

SharedTile TilesetEditee::add_tile(const SharedTile &tile)
{
    require_valid_tile_uuid(tile->uuid);

    _editee->body.tiles.push_back(tile);
    _tile_map[tile->uuid] = tile;
    tile_created(tile);
    return tile;
}

bool TilesetEditee::check_uuid(const PyEngine::UUID &uuid)
{
    if (uuid.is_nil()) {
        return false;
    }
    return (_tile_map.find(uuid) == _tile_map.end());
}

void TilesetEditee::delete_tile(const SharedTile &tile)
{
    std::vector<SharedTile> &tiles = _editee->body.tiles;
    auto it = std::find(tiles.begin(), tiles.end(), tile);
    if (it != tiles.end()) {
        tile_deleted(tile);
        tiles.erase(it);
        _tile_map.erase(tile->uuid);
    } else {
        throw std::logic_error("Attempt to delete foreign tile.");
    }
}

SharedTile TilesetEditee::duplicate_tile(
    const SharedTile &src,
    const PyEngine::UUID &uuid,
    bool rewrite_references_to_self)
{
    require_valid_tile_uuid(uuid);
    assert(src);
    TileData *new_tile = new TileData();
    *new_tile = *src;
    new_tile->uuid = uuid;
    // FIXME: apply rewrite_references_to_self
    return add_tile(std::shared_ptr<TileData>(new_tile));
}

SharedTile TilesetEditee::find_tile(const PyEngine::UUID &uuid) const
{
    auto it = _tile_map.find(uuid);
    if (it == _tile_map.end()) {
        return nullptr;
    } else {
        return (*it).second;
    }
}

SharedTile TilesetEditee::new_tile(const PyEngine::UUID &uuid)
{
    require_valid_tile_uuid(uuid);
    TileData *new_tile = new TileData();
    new_tile->uuid = uuid;
    new_tile->display_name = uuid.to_string();
    return add_tile(std::shared_ptr<TileData>(new_tile));
}

void TilesetEditee::set_name(const std::string &name)
{
    _name = name;
}

void TilesetEditee::set_author(const std::string &value)
{
    if (_editee->header.author == value) {
        return;
    }
    _editee->header.author = value;
    changed();
}

void TilesetEditee::set_description(const std::string &value)
{
    if (_editee->header.description == value) {
        return;
    }
    _editee->header.description = value;
    changed();
}

void TilesetEditee::set_display_name(const std::string &value)
{
    if (_editee->header.display_name == value) {
        return;
    }
    _editee->header.display_name = value;
    changed();
}

void TilesetEditee::set_license(const std::string &value)
{
    if (_editee->header.license == value) {
        return;
    }
    _editee->header.license = value;
    changed();
}

void TilesetEditee::set_version(const std::string &value)
{
    if (_editee->header.version == value) {
        return;
    }
    _editee->header.version = value;
    changed();
}

void TilesetEditee::set_tile_actor(const SharedTile &tile, bool value)
{
    if (tile->is_actor == value) {
        return;
    }
    tile->is_actor = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_blocking(const SharedTile &tile, bool value)
{
    if (tile->is_blocking == value) {
        return;
    }
    tile->is_blocking = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_cell_stamp(
    const SharedTile &tile,
    const CellStamp &stamp)
{
    tile->stamp = stamp;
    tile_changed(tile);
}

void TilesetEditee::set_tile_destructible(const SharedTile &tile, bool value)
{
    if (tile->is_destructible == value) {
        return;
    }
    tile->is_destructible = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_display_name(
        const SharedTile &tile,
        const std::string &value)
{
    if (tile->display_name == value) {
        return;
    }
    if (value == "") {
        throw std::invalid_argument("Tile display name must not be empty.");
    }
    tile->display_name = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_edible(const SharedTile &tile, bool value)
{
    if (tile->is_edible == value) {
        return;
    }
    tile->is_edible = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_gravity_affected(const SharedTile &tile, bool value)
{
    if (tile->is_gravity_affected == value) {
        return;
    }
    tile->is_gravity_affected = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_movable(const SharedTile &tile, bool value)
{
    if (tile->is_movable == value) {
        return;
    }
    tile->is_movable = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_roll_radius(
        const SharedTile &tile,
        float value)
{
    if (tile->roll_radius == value) {
        return;
    }
    if (value < 0) {
        value = 0;
    } else if (value > 1) {
        value = 1;
    }
    tile->roll_radius = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_rollable(const SharedTile &tile, bool value)
{
    if (tile->is_rollable == value) {
        return;
    }
    tile->is_rollable = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_sticky(const SharedTile &tile, bool value)
{
    if (tile->is_sticky == value) {
        return;
    }
    tile->is_sticky = value;
    tile_changed(tile);
}

void TilesetEditee::set_tile_temp_coefficient(
        const SharedTile &tile,
        float value)
{
    if (tile->temp_coefficient == value) {
        return;
    }
    if (value <= 0) {
        return;
    } else if (value > 100) {
        value = 100;
    }
    tile->temp_coefficient = value;
    tile_changed(tile);
}

void TilesetEditee::tile_changed(const SharedTile &tile)
{
    _tile_changed(this, tile);
    changed();
}
