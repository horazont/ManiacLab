#include "tileset.hpp"

#include <stdexcept>

#include "game_object.hpp"

/* TilesetTileInfo */

TilesetTileInfo::TilesetTileInfo(
        QUuid id,
        const std::string &display_name,
        const std::string &description):
    id(id),
    display_name(display_name),
    description(description)
{

}

TilesetTileInfo::~TilesetTileInfo()
{

}

Tileset::~Tileset()
{

}

std::unique_ptr<GameObject> Tileset::construct_tile(
        const QUuid &id,
        Level &level,
        const TileArgv &argv)
{
    auto iter = m_tile_map.find(id);
    if (iter == m_tile_map.end()) {
        return nullptr;
    }

    return construct_tile(*iter->second, level, argv);
}
