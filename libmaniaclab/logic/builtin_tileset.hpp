#ifndef ML_BUILTIN_TILESET_H
#define ML_BUILTIN_TILESET_H

#include "logic/tileset.hpp"


namespace std {
template<>
struct hash<::QUuid> {
    std::size_t operator()(const ::QUuid &value) const
    {
        return qHash(value);
    }
};

}


class BuiltInTileset: public AbstractTileset
{
private:
    using TileConstructor = std::function<std::unique_ptr<GameObject>(Level &level)>;
    static const std::unordered_map<QUuid, const TileData*> m_tile_info;
    static const std::unordered_map<QUuid, TileConstructor> m_tile_constructors;

    // AbstractTileset interface
public:
    std::unique_ptr<GameObject> construct_tile(
            const QUuid &uuid,
            Level &level) override;

};

#endif
