#ifndef _ML_TILE_OPERATIONS_H
#define _ML_TILE_OPERATIONS_H

#include "io/TilesetData.hpp"

#include "TilesetOperations.hpp"

class OpSetTileCellStamp: public TilesetOperation
{
public:
    OpSetTileCellStamp(
        TilesetEditee *editee,
        const SharedTile &tile,
        const CellStamp &new_stamp);

private:
    SharedTile _tile;
    CellStamp _new_stamp;
    CellStamp _old_stamp;

public:
    void execute() override;
    void undo() override;

};

#endif
