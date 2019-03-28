#include "builtin_tileset.hpp"

#include <functional>
#include <memory>

#include "logic/bomb_object.hpp"
#include "logic/wall_object.hpp"

static std::unique_ptr<GameObject> construct_bomb(
        Level &level,
        const TileArgv &argv)
{
    return std::make_unique<BombObject>(level);
}

static std::unique_ptr<GameObject> construct_safe_wall_square(
        Level &level,
        const TileArgv &argv)
{
    return std::make_unique<SafeWallObject>(level);
}

static std::unique_ptr<GameObject> construct_safe_wall_round(
        Level &level,
        const TileArgv &argv)
{
    return std::make_unique<RoundSafeWallObject>(level);
}

BuiltInTileset::BuiltInTileset()
{
    emplace_tile(
                QUuid("{4695311f-834b-4a04-9c4b-f8efac161d31}"),
                construct_bomb,
                "Bomb", ""
                );
    emplace_tile(
                QUuid("{d0586ef6-4feb-43ed-8a83-63c2bb0b5bcb}"),
                construct_safe_wall_square,
                "Wall (square, massive)", ""
                );
    emplace_tile(
                QUuid("{b93e0aea-ef07-4c43-a045-9b4c982e5c74}"),
                construct_safe_wall_round,
                "Wall (round, massive)", ""
                );
}

std::unique_ptr<GameObject> BuiltInTileset::construct_tile(
        const TilesetTileInfo &tile,
        Level &level,
        const TileArgv &argv)
{
    auto iter = m_constructors.find(tile.id);
    if (iter == m_constructors.end()) {
        return nullptr;
    }

    return iter->second(level, argv);
}

const BuiltInTileset &BuiltInTileset::instance()
{
    static const BuiltInTileset tileset;
    return tileset;
}
