#include "tileset.hpp"

#include <stdexcept>

#include "game_object.hpp"

TilesetTile::TilesetTile(QUuid id, std::string_view display_name):
    m_id(id),
    m_display_name(display_name)
{

}

TilesetTile::~TilesetTile()
{

}


Tileset::~Tileset()
{

}

std::unique_ptr<GameObject> Tileset::make_tile(const QUuid &id, Level &level, const TileArgv &argv) const
{
    for (auto &tiledef: tiles()) {
        if (tiledef->id() == id) {
            return tiledef->instantiate(level, argv);
        }
    }
    return nullptr;
}

std::unique_ptr<GameObject> SimpleTileset::make_tile(const QUuid &id, Level &level, const TileArgv &argv) const
{
    auto iter = m_tile_map.find(id);
    if (iter == m_tile_map.end()) {
        return nullptr;
    }
    return iter->second->instantiate(level, argv);
}

const TilesetTileContainer &SimpleTileset::tiles() const
{
    return m_tiles;
}
