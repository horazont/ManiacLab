/**********************************************************************
File name: TileOperations.cpp
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
