#ifndef ML_BUILTIN_TILESET_H
#define ML_BUILTIN_TILESET_H

#include <functional>

#include "logic/tileset.hpp"

class Level;

class BuiltInTile: public TilesetTile
{
public:
    explicit BuiltInTile(
            std::function<std::unique_ptr<GameObject>(Level&)> ctor,
            QUuid id,
            std::string_view display_name
            );

private:
    std::function<std::unique_ptr<GameObject>(Level&)> m_ctor;

public:
    std::unique_ptr<GameObject> instantiate(
                Level &level,
                const TileArgv &argv) const override;
};


std::unique_ptr<SimpleTileset> make_builtin_tileset();

#endif
