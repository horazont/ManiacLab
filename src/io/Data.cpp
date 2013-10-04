/**********************************************************************
File name: Data.cpp
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
#include "Data.hpp"

#include <CEngine/IO/Log.hpp>

#include <structstream/structstream.hpp>

#include "StructstreamIntf.hpp"

using namespace StructStream;
using namespace PyEngine;

/* free functions */

FileType get_type_from_tree(const ContainerHandle &root)
{
    if (root->children_begin() == root->children_end()) {
        return FT_EMPTY;
    }
    ContainerHandle first_child = std::dynamic_pointer_cast<Container>(
        *root->children_begin());

    if (!first_child) {
        return FT_NO_MANIACLAB;
    }

    switch (first_child->id()) {
    case SSID_TILESET_HEADER:
        return FT_TILESET;
    case SSID_LEVEL_COLLECTION_HEADER:
        return FT_LEVEL_COLLECTION;
    default:
        return FT_NO_MANIACLAB;
    };
}

std::pair<ContainerHandle, FileType> load_header_from_stream(
    const StreamHandle &stream)
{
    IOIntfHandle io(new PyEngineStream(stream));
    ContainerHandle root;
    try {
        root = bitstream_to_tree(
            io, maniac_lab_registry,
            FromBitstream::UnknownHashFunction
            );
    } catch (const UnsupportedInput &err) {
        PyEngine::log->getChannel("io")->log(Error)
            << "Input file uses unsuppored features: " << err.what() << submit;
        return std::make_pair(nullptr, FT_UNSUPPORTED);
    } catch (const FormatError &err) {
        PyEngine::log->getChannel("io")->log(Error)
            << "Input file is not a StructStream (format errors): " << err.what() << submit;
        return std::make_pair(nullptr, FT_NO_STRUCTSTREAM);
    } catch (const std::runtime_error &err) {
        PyEngine::log->getChannel("io")->log(Error)
            << "Input file is not a StructStream: " << err.what() << submit;
        return std::make_pair(nullptr, FT_NO_STRUCTSTREAM);
    }

    FileType filet = get_type_from_tree(root);
    if (filet <= FT_NO_MANIACLAB) {
        return std::make_pair(nullptr, filet);
    }

    return std::make_pair(root, filet);
}
