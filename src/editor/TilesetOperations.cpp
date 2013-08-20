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

const bool &OpSetTileDisplayName::get_value() const
{
    return _tile->is_blocking;
}

void OpSetTileDisplayName::set_value(const bool &value)
{
    _tileset->set_tile_blocking(_tile, value);
}
