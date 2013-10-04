/**********************************************************************
File name: Data.hpp
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
#ifndef _ML_DATA_H
#define _ML_DATA_H

#include "LevelData.hpp"
#include "TilesetData.hpp"

enum FileType {
    FT_NO_STRUCTSTREAM,
    FT_EMPTY,
    FT_UNSUPPORTED,
    FT_NO_MANIACLAB,
    FT_LEVEL_COLLECTION,
    FT_TILESET
};

FileType get_type_from_tree(const StructStream::ContainerHandle &root);

std::pair<StructStream::ContainerHandle, FileType>
    load_header_from_stream(const PyEngine::StreamHandle &stream);


#endif
