#include "builtin_tileset.hpp"

#include <functional>
#include <memory>

#include "logic/bomb_object.hpp"
#include "logic/wall_object.hpp"
#include "logic/rock_object.hpp"
#include "logic/fan_object.hpp"
#include "logic/fog_object.hpp"


template <typename T>
std::function<std::unique_ptr<GameObject>(Level&)> make_simple_ctor()
{
    return [](Level &level){ return std::make_unique<T>(level); };
}


BuiltInTile::BuiltInTile(std::function<std::unique_ptr<GameObject> (Level &)> ctor, QUuid id, std::string_view display_name):
    TilesetTile(id, display_name),
    m_ctor(ctor)
{

}

std::unique_ptr<GameObject> BuiltInTile::instantiate(Level &level, const TileArgv&) const
{
    return m_ctor(level);
}


std::unique_ptr<SimpleTileset> make_builtin_tileset()
{
    auto result = std::make_unique<SimpleTileset>();
    result->emplace_tile<BuiltInTile>(
                make_simple_ctor<BombObject>(),
                QUuid("{4695311f-834b-4a04-9c4b-f8efac161d31}"),
                "Bomb"
                );
    result->emplace_tile<BuiltInTile>(
                make_simple_ctor<RockObject>(),
                QUuid("{31a2aba1-609e-4a4d-9246-4aaca26c8749}"),
                "Rock"
                );
    result->emplace_tile<BuiltInTile>(
                make_simple_ctor<SafeWallObject>(),
                QUuid("{1868c6b6-c535-4d6a-9a09-823903c5a4e3}"),
                "Safe wall (square)"
                );
    result->emplace_tile<BuiltInTile>(
                make_simple_ctor<RoundSafeWallObject>(),
                QUuid("{a02b52d3-0cf0-4847-8808-2dca7143d170}"),
                "Safe wall (round)"
                );
    return result;
}
