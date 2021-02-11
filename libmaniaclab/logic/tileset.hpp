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

using TileArgv = std::unordered_multimap<mlio::TileArgType, QVariant>;


class TilesetTile
{
public:
    TilesetTile() = delete;
    explicit TilesetTile(
            QUuid id,
            std::string_view display_name);
    virtual ~TilesetTile();

private:
    QUuid m_id;
    std::string m_display_name;

public:
    inline const QUuid &id() const {
        return m_id;
    }

    inline std::string_view display_name() const {
        return m_display_name;
    }

    virtual std::unique_ptr<GameObject> instantiate(
            Level &level,
            const TileArgv &argv) const = 0;

};

using TilesetTileHandle = std::unique_ptr<TilesetTile>;
using TilesetTileContainer = std::vector<TilesetTileHandle>;


class Tileset
{
public:
    Tileset() = default;
    Tileset(const Tileset &ref) = default;
    Tileset(Tileset &&src) = default;
    Tileset& operator=(const Tileset &ref) = default;
    Tileset& operator=(Tileset &&src) = default;
    virtual ~Tileset();

public:
    virtual std::unique_ptr<GameObject> make_tile(
            const QUuid &id,
            Level &level,
            const TileArgv &argv) const;

    virtual const TilesetTileContainer &tiles() const = 0;
};


class SimpleTileset: public Tileset
{
public:
    using Tileset::Tileset;

private:
    TilesetTileContainer m_tiles;
    std::unordered_map<QUuid, TilesetTile*> m_tile_map;

public:
    template <typename T, typename... arg_ts>
    T &emplace_tile(arg_ts&&... args)
    {
        std::unique_ptr<T> result = std::make_unique<T>(std::forward<arg_ts>(args)...);
        return register_tile(std::move(result));
    }

    template <typename T>
    T &register_tile(std::unique_ptr<T> &&src)
    {
        auto iter = m_tile_map.find(src->id());
        if (iter != m_tile_map.end()) {
            throw std::runtime_error("uuid already in use");
        }
        T &result_ref = *src;
        m_tiles.emplace_back(std::move(src));
        m_tile_map[result_ref.id()] = &result_ref;
        return result_ref;
    }

    std::unique_ptr<GameObject> make_tile(
                const QUuid &id,
                Level &level,
                const TileArgv &argv) const override;
    const TilesetTileContainer &tiles() const override;
};


#endif
