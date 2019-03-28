#ifndef ML_BUILTIN_TILESET_H
#define ML_BUILTIN_TILESET_H

#include <functional>

#include "logic/tileset.hpp"

class Level;

class BuiltInTileset: public Tileset
{
private:
    using TileConstructor = std::function<std::unique_ptr<GameObject>(Level &level, const TileArgv &argv)>;

protected:
    BuiltInTileset();

private:
    std::unordered_map<QUuid, TileConstructor> m_constructors;

    template <typename... arg_ts>
    TilesetTileInfo &emplace_tile(QUuid id, TileConstructor constructor, arg_ts... args)
    {
        TilesetTileInfo &tile = Tileset::emplace_tile<TilesetTileInfo>(id, std::forward<arg_ts>(args)...);
        m_constructors.emplace(id, std::move(constructor));
        return tile;
    }

protected:
    std::unique_ptr<GameObject> construct_tile(const TilesetTileInfo &tile,
                                               Level &level,
                                               const TileArgv &argv) override;

public:
    static const BuiltInTileset &instance();

};

#endif
