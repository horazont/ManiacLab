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

OpNewTile::OpNewTile(TilesetEditee *tileset,
                     const std::string &unique_name):
    TilesetOperation(tileset),
    _unique_name(unique_name),
    _tile(nullptr)
{

}

void OpNewTile::execute()
{
    if (_tile) {
        _tile = _tileset->add_tile(_tile);
    } else {
        _tile = _tileset->new_tile(_unique_name);
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
        const std::string &unique_name,
        bool rewrite_references_to_self):
    TilesetOperation(tileset),
    _src(src),
    _unique_name(unique_name),
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
            _unique_name,
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

/* OpSetTileActor */

OpSetTileActor::OpSetTileActor(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileActor::get_value() const
{
    return _tile->is_actor;
}

void OpSetTileActor::set_value(const bool &value)
{
    _tileset->set_tile_actor(_tile, value);
}

/* OpSetTileBlocking */

OpSetTileBlocking::OpSetTileBlocking(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileBlocking::get_value() const
{
    return _tile->is_blocking;
}

void OpSetTileBlocking::set_value(const bool &value)
{
    _tileset->set_tile_blocking(_tile, value);
}

/* OpSetTileDestructible */

OpSetTileDestructible::OpSetTileDestructible(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileDestructible::get_value() const
{
    return _tile->is_destructible;
}

void OpSetTileDestructible::set_value(const bool &value)
{
    _tileset->set_tile_destructible(_tile, value);
}

/* OpSetTileEdible */

OpSetTileEdible::OpSetTileEdible(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileEdible::get_value() const
{
    return _tile->is_edible;
}

void OpSetTileEdible::set_value(const bool &value)
{
    _tileset->set_tile_edible(_tile, value);
}

/* OpSetTileGravityAffected */

OpSetTileGravityAffected::OpSetTileGravityAffected(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileGravityAffected::get_value() const
{
    return _tile->is_gravity_affected;
}

void OpSetTileGravityAffected::set_value(const bool &value)
{
    _tileset->set_tile_gravity_affected(_tile, value);
}

/* OpSetTileMovable */

OpSetTileMovable::OpSetTileMovable(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileMovable::get_value() const
{
    return _tile->is_movable;
}

void OpSetTileMovable::set_value(const bool &value)
{
    _tileset->set_tile_movable(_tile, value);
}

/* OpSetTileRollable */

OpSetTileRollable::OpSetTileRollable(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileRollable::get_value() const
{
    return _tile->is_rollable;
}

void OpSetTileRollable::set_value(const bool &value)
{
    _tileset->set_tile_rollable(_tile, value);
}

/* OpSetTileSticky */

OpSetTileSticky::OpSetTileSticky(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value):
    OpSetTileAttribute<bool>(tileset, tile, value)
{

}

const bool &OpSetTileSticky::get_value() const
{
    return _tile->is_sticky;
}

void OpSetTileSticky::set_value(const bool &value)
{
    _tileset->set_tile_sticky(_tile, value);
}

/* OpSetTileRollRadius */

OpSetTileRollRadius::OpSetTileRollRadius(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const float &value):
    OpSetTileAttribute<float>(tileset, tile, value)
{

}

const float &OpSetTileRollRadius::get_value() const
{
    return _tile->roll_radius;
}

void OpSetTileRollRadius::set_value(const float &value)
{
    _tileset->set_tile_roll_radius(_tile, value);
}

/* OpSetTileTempCoefficient */

OpSetTileTempCoefficient::OpSetTileTempCoefficient(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const float &value):
    OpSetTileAttribute<float>(tileset, tile, value)
{

}

const float &OpSetTileTempCoefficient::get_value() const
{
    return _tile->roll_radius;
}

void OpSetTileTempCoefficient::set_value(const float &value)
{
    _tileset->set_tile_roll_radius(_tile, value);
}

