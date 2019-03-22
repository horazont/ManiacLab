#include "builtin_tileset.hpp"

#include <functional>
#include <memory>

#include "logic/bomb_object.hpp"

/* BuiltInTileset */

std::unique_ptr<GameObject> BuiltInTileset::construct_tile(const QUuid &uuid,
                                                           Level &level)
{
    auto iter = m_tile_constructors.find(uuid);
    if (iter == m_tile_constructors.end()) {
        return nullptr;
    }
    return iter->second(level);
}

static std::unique_ptr<BombObject> make_bomb_object(Level &level)
{
    return std::make_unique<BombObject>(level);
}


const std::unordered_map<QUuid, BuiltInTileset::TileConstructor> BuiltInTileset::m_tile_constructors(
    {
        {QUuid("{3d070d1d-8a5d-428b-97f2-ac1b19e65316}"), &make_bomb_object}
    });
