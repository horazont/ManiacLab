#include "IOUtils.hpp"

void load_cell_stamp(PyEngine::StreamHandle &instream, BoolCellStamp stamp)
{
    uint8_t subdivisions = instream->readUInt8();
    if (subdivisions != subdivision_count)
    {
	throw PyEngine::Exception("incompatible stamp format (subdivision count mismatch)");
    }
    static_assert(subdivision_count <= 8, "We cannot handle more than one byte per row yet");

    bool *stamp_cell = stamp;
    for (int row = 0; row < subdivision_count; row++) {
	uint8_t row_data = instream->readUInt8();
	for (int cell = 0; cell < subdivision_count; cell++) {
	    *stamp_cell = ((1 << cell) & row_data) != 0;
	    stamp_cell++;
	}
    }
}

void save_cell_stamp(PyEngine::StreamHandle &outstream, const BoolCellStamp &stamp)
{
    outstream->writeUInt8(subdivision_count);
    static_assert(subdivision_count <= 8, "We cannot handle more than one byte per row yet");
    const bool *stamp_cell = stamp;
    for (int row = 0; row < subdivision_count; row++) {
	uint8_t row_data = 0;
	for (int cell = 0; cell < subdivision_count; cell++) {
	    if (*stamp_cell) {
		row_data |= (1 << cell);
	    }
	}
	outstream->writeUInt8(row_data);
    }
}
 
