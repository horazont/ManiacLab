#ifndef _ML_TILESET_DATA_H
#define _ML_TILESET_DATA_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <CEngine/IO/Stream.hpp>

#include <structstream/static.hpp>

#include "Common.hpp"

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

    float roll_radius;
    float temp_coefficient;

    /* TODO: ca stamp */
    VisualData default_visual;
    std::vector<TileVisualData> additional_visuals;
};

struct TilesetData
{
    std::string display_name;
    std::string unique_name;
    std::string description;

    std::string author;
    std::string license;
    std::string version;

    std::vector<TileData> tiles;
};

std::unique_ptr<TilesetData> load_tileset_from_stream(
    const PyEngine::StreamHandle &stream);

void save_tileset_to_stream(
    const TilesetData &tileset,
    const PyEngine::StreamHandle &stream);

#endif
