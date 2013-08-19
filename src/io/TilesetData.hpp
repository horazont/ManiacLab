/**********************************************************************
File name: TilesetData.hpp
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
#ifndef _ML_TILESET_DATA_H
#define _ML_TILESET_DATA_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <CEngine/IO/Stream.hpp>

#include <structstream/static.hpp>
#include <structstream/streaming_base.hpp>

#include "Common.hpp"

const StructStream::ID SSID_TILESET_HEADER = 0x4d4c5473;

class TileVisualRecord;
typedef std::shared_ptr<TileVisualRecord> TileVisualRecordHandle;

struct ImageData
{
    ImageData& operator=(const TileVisualRecord &rec);

    uint16_t width, height;
    TileVisualFormat format;
    std::basic_string<uint8_t> data;
    float duration;
};

struct FrameData
{
    ImageData image;
    float duration;

    void image_data_from_record(const TileVisualRecordHandle &rec);
    TileVisualRecordHandle image_data_to_record(StructStream::ID id) const;
};

struct VisualData
{
    std::vector<FrameData> frames;
};

struct TileVisualMatchData
{
    bool relevant;
    std::string unique_name;
};

struct TileVisualData
{
    TileVisualMatchData top, left, right, bottom;
    VisualData visual;
};

struct TileData
{
    std::string unique_name;
    std::string display_name;

    bool is_rollable;
    bool is_sticky;
    bool is_edible;
    bool is_gravity_affected;
    bool is_blocking;

    float roll_radius;
    float temp_coefficient;

    /* TODO: ca stamp */
    VisualData default_visual;
    std::vector<TileVisualData> additional_visuals;
};

struct TilesetHeaderData
{
    std::string display_name;
    std::string unique_name;
    std::string description;

    std::string author;
    std::string license;
    std::string version;
};

struct TilesetBodyData
{
    std::vector<std::shared_ptr<TileData>> tiles;
};

struct TilesetData
{
    TilesetHeaderData header;
    TilesetBodyData body;
};

typedef std::shared_ptr<TilesetData> SharedTileset;
typedef std::shared_ptr<TileData> SharedTile;

std::pair<StructStream::StreamSink, std::unique_ptr<TilesetData>>
    create_tileset_sink();

std::unique_ptr<TilesetData> load_tileset_from_tree(
    const StructStream::ContainerHandle &root);

std::unique_ptr<TilesetData> load_tileset_from_stream(
    const PyEngine::StreamHandle &stream);

std::unique_ptr<TilesetHeaderData> load_tileset_header_from_stream(
    const PyEngine::StreamHandle &stream);


void save_tileset_to_stream(
    const TilesetData &tileset,
    const PyEngine::StreamHandle &stream);

#endif
