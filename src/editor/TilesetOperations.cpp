/**********************************************************************
File name: TilesetOperations.cpp
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
#include "TilesetOperations.hpp"

/* TilesetOperation */

TilesetOperation::TilesetOperation(TilesetEditee *tileset):
    Operation(),
    _tileset(tileset)
{

}

/* OpNewTile */

OpNewTile::OpNewTile(TilesetEditee *tileset):
    TilesetOperation(tileset),
    _tile(nullptr)
{

}

void OpNewTile::execute()
{
    if (_tile) {
        _tile = _tileset->add_tile(_tile);
    } else {
        _tile = _tileset->new_tile(
            PyEngine::UUID::with_generator<PyEngine::uuid_random>());
    }
}

void OpNewTile::undo()
{
    _tileset->delete_tile(_tile);
}

/* OpDuplicateTile */

OpDuplicateTile::OpDuplicateTile(
        TilesetEditee *tileset,
        const SharedTile &src,
        bool rewrite_references_to_self):
    TilesetOperation(tileset),
    _src(src),
    _rewrite_references_to_self(rewrite_references_to_self),
    _tile(nullptr)
{

}

void OpDuplicateTile::execute()
{
    if (_tile) {
        _tile = _tileset->add_tile(_tile);
    } else {
        _tile = _tileset->duplicate_tile(
            _src,
            PyEngine::UUID::with_generator<PyEngine::uuid_random>(),
            _rewrite_references_to_self);
    }
}

void OpDuplicateTile::undo()
{
    _tileset->delete_tile(_tile);
}

/* OpDeleteTile */

OpDeleteTile::OpDeleteTile(TilesetEditee *tileset,
                           const SharedTile &tile):
    TilesetOperation(tileset),
    _tile(tile)
{

}

void OpDeleteTile::execute()
{
    _tileset->delete_tile(_tile);
}

void OpDeleteTile::undo()
{
    _tile = _tileset->add_tile(_tile);
}

/* OpSetTileDisplayName */

OpSetTileDisplayName::OpSetTileDisplayName(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const std::string &value):
    OpSetTileAttribute<std::string>(tileset, tile, value)
{

}

const std::string &OpSetTileDisplayName::get_value() const
{
    return _tile->display_name;
}

void OpSetTileDisplayName::set_value(const std::string &value)
{
    _tileset->set_tile_display_name(_tile, value);
}
