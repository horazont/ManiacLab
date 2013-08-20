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

