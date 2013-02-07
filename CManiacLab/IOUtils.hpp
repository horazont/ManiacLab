#ifndef _ML_IO_UTILS_H
#define _ML_IO_UTILS_H

#include <CEngine/IO/Stream.hpp>

#include "Stamp.hpp"

void load_cell_stamp(PyEngine::StreamHandle &instream, BoolCellStamp stamp);
void save_cell_stamp(PyEngine::StreamHandle &outstream, const BoolCellStamp &stamp);

#endif
