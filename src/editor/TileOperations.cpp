#include "TileOperations.hpp"

/* OpSetTileCellStamp */

OpSetTileCellStamp::OpSetTileCellStamp(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const CellStamp &stamp):
    TilesetOperation(tileset),
    _tile(tile),
    _new_stamp(stamp)
{

}

void OpSetTileCellStamp::execute()
{
    _old_stamp = _tile->stamp;
    _tileset->set_tile_cell_stamp(_tile, _new_stamp);
}

void OpSetTileCellStamp::undo()
{
    _tileset->set_tile_cell_stamp(_tile, _old_stamp);
}
