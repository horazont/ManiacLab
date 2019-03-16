#ifndef ML_TILESET_H
#define ML_TILESET_H

#include <functional>
#include <memory>
#include <unordered_map>

#include <QUuid>


class GameObject;
class Level;
struct TileData;


class AbstractTileset
{
public:
    virtual ~AbstractTileset();

public:
    virtual std::unique_ptr<GameObject> construct_tile(const QUuid &uuid,
                                                       Level &level) = 0;
    virtual const TileData &get_tile_info(const QUuid &uuid) = 0;

};

#endif
