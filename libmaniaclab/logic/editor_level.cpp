#include "editor_level.hpp"

#include <iostream>

#include <ffengine/io/log.hpp>

#include "maniaclab.pb.h"

static io::Logger &logger = io::logging().get_logger("maniaclib.editor_level");

static float get_initial_temperature(const TileArgv &argv) {
    auto iter = argv.find(mlio::INITIAL_TEMPERATURE);
    if (iter == argv.end()) {
        return 1.0f;
    }
    {
        const float *value = std::get_if<float>(&iter->second);
        if (value != nullptr) {
            return *value;
        }
    }
    {
        const double *value = std::get_if<double>(&iter->second);
        if (value != nullptr) {
            return *value;
        }
    }
    return 1.0f;
}

EditorLevel::EditorLevel():
    m_level(level_width, level_height)
{

}

bool EditorLevel::place_object(const Tileset &tileset, QUuid id, const TileArgv &argv, CoordInt x, CoordInt y)
{
    auto tile = tileset.make_tile(id, m_level, argv);
    if (tile == nullptr) {
        return false;
    }
    m_level.place_object(std::move(tile), x, y, get_initial_temperature(argv));
    m_tile_placements[CoordPair(x, y)] = TileTemplate{id, argv};
    return true;
}

/* FIXME: deduplicate code with level.cpp */

static std::tuple<bool, TileArgValue> deprotobufferize_arg_value(
        const mlio::TileArg &arg)
{
    switch (arg.value_case()) {
    case mlio::TileArg::kSval:
        return std::make_tuple(true, TileArgValue(arg.sval()));
    case mlio::TileArg::kFval:
        return std::make_tuple(true, TileArgValue(arg.fval()));
    case mlio::TileArg::kDval:
        return std::make_tuple(true, TileArgValue(arg.dval()));
    case mlio::TileArg::kIval:
        return std::make_tuple(true, TileArgValue(arg.ival()));
    case mlio::TileArg::kUival:
        return std::make_tuple(true, TileArgValue(arg.uival()));
    case mlio::TileArg::kBval:
        return std::make_tuple(true, TileArgValue(arg.bval()));
    case mlio::TileArg::kBinval:
        return std::make_tuple(true, TileArgValue(arg.binval()));
    case mlio::TileArg::kPval:
    {
        const auto pval = arg.pval();
        return std::make_tuple(
                    true,
                    TileArgValue(CoordPair(pval.x(), pval.y()))
                    );
    }
    default:
        return std::make_tuple(false, TileArgValue(0));
    }
}

bool EditorLevel::load(const Tileset &tileset, std::istream &in)
{
    mlio::Level level;
    if (!level.ParseFromIstream(&in)) {
        logger.logf(io::LOG_ERROR, "protobuf failed to parse");
        return false;
    }

    m_level.clear();
    m_tile_placements.clear();
    TileArgv argv;
    for (int i = 0; i < level.cells_size(); ++i) {
        const CoordInt y = i / level_width;
        const CoordInt x = i % level_width;
        if (y >= level_height || x >= level_width) {
            logger.logf(io::LOG_ERROR, "level size out of bounds");
            m_level.clear();
            m_tile_placements.clear();
            return false;
        }

        const auto cell = level.cells(i);
        if (!cell.tile().size()) {
            // empty cell
            continue;
        }

        const QUuid id(cell.tile().c_str());
        argv.clear();
        for (const auto &arg: cell.argv()) {
            auto [ok, value] = deprotobufferize_arg_value(arg);
            if (ok) {
                argv.emplace(arg.type(), std::move(value));
            }
        }
        if (!place_object(tileset, id, argv, x, y)) {
            logger.logf(io::LOG_ERROR, "failed to instantiate tile %s from saved file", id.toString().data());
            m_level.clear();
            m_tile_placements.clear();
            return false;
        }
    }

    return true;
}

bool protobufferize_arg_value(mlio::TileArg &dest,
                              const TileArgValue &value)
{
    {
        auto raw = std::get_if<std::int64_t>(&value);
        if (raw != nullptr) {
            dest.set_ival(*raw);
            return true;
        }
    }
    {
        auto raw = std::get_if<std::uint64_t>(&value);
        if (raw != nullptr) {
            dest.set_uival(*raw);
            return true;
        }
    }
    {
        auto raw = std::get_if<float>(&value);
        if (raw != nullptr) {
            dest.set_fval(*raw);
            return true;
        }
    }
    {
        auto raw = std::get_if<double>(&value);
        if (raw != nullptr) {
            dest.set_dval(*raw);
            return true;
        }
    }
    {
        auto raw = std::get_if<std::string>(&value);
        if (raw != nullptr) {
            dest.set_sval(raw->data(), raw->size());
            return true;
        }
    }
    {
        auto raw = std::get_if<std::basic_string<std::byte>>(&value);
        if (raw != nullptr) {
            dest.set_binval(raw->data(), raw->size());
            return true;
        }
    }
    {
        auto raw = std::get_if<bool>(&value);
        if (raw != nullptr) {
            dest.set_bval(*raw);
            return true;
        }
    }
    {
        auto raw = std::get_if<CoordPair>(&value);
        if (raw != nullptr) {
            auto pair = std::make_unique<mlio::CoordPair>();
            pair->set_x(raw->x);
            pair->set_y(raw->y);
            dest.set_allocated_pval(pair.release());
            return true;
        }
    }
    return false;
}

void EditorLevel::save(std::ostream &out)
{
    mlio::Level level;
    for (CoordInt y = 0; y < level_height; ++y) {
        for (CoordInt x = 0; x < level_width; ++x) {
            auto cell = level.add_cells();
            auto iter = m_tile_placements.find(CoordPair(x, y));
            if (iter != m_tile_placements.end()) {
                TileTemplate &tile_template = iter->second;
                cell->set_tileset(0);
                cell->set_tile(tile_template.id.toByteArray().constData());
                for (auto arg_iter: tile_template.argv) {
                    auto arg = cell->add_argv();
                    arg->set_type(arg_iter.first);
                    auto value = arg_iter.second;
                    protobufferize_arg_value(*arg, value);
                }
            }
        }
    }
    level.SerializeToOstream(&out);
}
