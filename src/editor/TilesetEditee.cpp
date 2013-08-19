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

TilesetEditee::TilesetEditee(const SharedTileset &editee):
    _editee(editee),
    _changed(),
    _tile_changed(),
    _tile_created(),
    _tile_deleted()
{
    for (auto& tile: tiles()) {
        _tile_map[tile->unique_name] = tile;
    }
}

void TilesetEditee::changed()
{
    _changed(this);
}

void TilesetEditee::require_unique_tile_name(
        const std::string &unique_name)
{
    if (_tile_map.find(unique_name) != _tile_map.end()) {
        throw std::runtime_error("Cannot create two tiles with same unique name.");
    }
}

void TilesetEditee::tile_changed(const SharedTile &tile)
{
    _tile_changed(this, tile);
    changed();
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

SharedTile TilesetEditee::add_tile(std::unique_ptr<TileData> &&tile)
{
    require_unique_tile_name(tile->unique_name);

    SharedTile new_tile(tile.release());
    _editee->body.tiles.push_back(new_tile);
    _tile_map[new_tile->unique_name] = new_tile;
    tile_created(new_tile);
    return new_tile;
}

void TilesetEditee::delete_tile(const SharedTile &tile)
{
    std::vector<SharedTile> &tiles = _editee->body.tiles;
    auto it = std::find(tiles.begin(), tiles.end(), tile);
    if (it != tiles.end()) {
        tile_deleted(tile);
        tiles.erase(it);
        _tile_map.erase(tile->unique_name);
    } else {
        throw std::logic_error("Attempt to delete foreign tile.");
    }
}

SharedTile TilesetEditee::duplicate_tile(
        const SharedTile &src,
        const std::string &unique_name)
{
    TileData *new_tile = new TileData();
    *new_tile = *src;
    new_tile->unique_name = unique_name;
    return add_tile(std::unique_ptr<TileData>(new_tile));
}

SharedTile TilesetEditee::new_tile(const std::string &unique_name)
{
    TileData *new_tile = new TileData();
    new_tile->unique_name = unique_name;
    return add_tile(std::unique_ptr<TileData>(new_tile));
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