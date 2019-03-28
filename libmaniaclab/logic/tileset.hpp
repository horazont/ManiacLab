#ifndef ML_TILESET_H
#define ML_TILESET_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "sigc++/sigc++.h"

#include <QVariant>
#include <QUuid>

#include "maniaclab.pb.h"

class Level;
struct GameObject;

namespace std {
template<>
struct hash<::QUuid> {
    std::size_t operator()(const ::QUuid &value) const
    {
        return qHash(value);
    }
};

/* template<>
struct hash<::mlio::TileArgType> {
    std::size_t operator()(::mlio::TileArgType value) const
    {
        static_assert(sizeof(::mlio::TileArgType) == sizeof(std::int32_t),
                      "unexpected enum size");
        return hash<int32_t>()(static_cast<int32_t>(value));
    }
}; */

}

struct TilesetTileInfo
{
    TilesetTileInfo() = default;
    TilesetTileInfo(QUuid id,
                    const std::string &display_name,
                    const std::string &description);
    TilesetTileInfo(const TilesetTileInfo &ref) = delete;
    TilesetTileInfo& operator=(const TilesetTileInfo &ref) = delete;
    TilesetTileInfo(TilesetTileInfo &&src) = default;
    TilesetTileInfo& operator=(TilesetTileInfo &&src) = default;
    virtual ~TilesetTileInfo();

    QUuid id;
    // TODO: i18n
    std::string display_name;
    std::string description;
};

using TilesetTileHandle = std::unique_ptr<TilesetTileInfo>;
using TilesetTileContainer = std::vector<TilesetTileHandle>;

using TileArgv = std::unordered_multimap<mlio::TileArgType, QVariant>;

class Tileset
{
public:
    virtual ~Tileset();

private:
    TilesetTileContainer m_tiles;
    std::unordered_map<QUuid, TilesetTileHandle> m_tile_map;

protected:
    void register_tile(std::unique_ptr<TilesetTileInfo> &&tile) {
        if (m_tile_map.find(tile->id) != m_tile_map.end()) {
            throw std::invalid_argument("duplicate tile UUID: "+tile->id.toString().toStdString());
        }
        m_tiles.emplace_back(std::move(tile));
    }

    template <typename T, typename... arg_ts>
    T &emplace_tile(arg_ts... args) {
        auto tile_specific = std::make_unique<T>(std::forward<arg_ts>(args)...);
        T &result = *tile_specific;
        TilesetTileHandle tile = std::move(tile_specific);
        register_tile(std::move(tile));
        return result;
    }

    virtual std::unique_ptr<GameObject> construct_tile(const TilesetTileInfo &info,
                                                       Level &level,
                                                       const TileArgv &argv) = 0;

public:
    inline const TilesetTileContainer &tiles() const {
        return m_tiles;
    }

    std::unique_ptr<GameObject> construct_tile(const QUuid &id,
                                               Level &level,
                                               const TileArgv &argv);
};

#endif
