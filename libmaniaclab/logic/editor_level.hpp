#ifndef ML_EDITOR_LEVEL_H
#define ML_EDITOR_LEVEL_H

#include "level.hpp"
#include "tileset.hpp"


struct TileTemplate
{
    /* Tileset *tileset; */
    QUuid id;
    TileArgv argv;
};


class EditorLevel
{
public:
    EditorLevel();

private:
    Level m_level;
    std::unordered_map<CoordPair, TileTemplate> m_tile_placements;

public:
    inline Level &level() {
        return m_level;
    }

    inline const Level &level() const {
        return m_level;
    }

    bool place_object(const Tileset &tileset,
                      QUuid id,
                      const TileArgv &argv,
                      CoordInt x, CoordInt y);

    bool load(const Tileset &tileset, std::istream &in);
    void save(std::ostream &out);

};

#endif
