#ifndef _ML_SAFE_WALL_OBJECT_H
#define _ML_SAFE_WALL_OBJECT_H

#include "GameObject.hpp"
#include "MetatextureView.hpp"

class SafeWallView: public MetatextureView
{
public:
    SafeWallView(TileMaterialManager &matman);

};

class WallObject: public GameObject
{
public:
    WallObject(ObjectInfo &info, Level *level);

};

class SafeWallObject: public WallObject
{
public:
    SafeWallObject(Level *level);

public:
    void setup_view(TileMaterialManager &matman) override;

};

std::unique_ptr<ObjectView> wall_view(
    TileMaterialManager &matman,
    Level &level,
    const CoordPair &cell,
    const std::string &prefix);

#endif
