#ifndef _ML_SAFE_WALL_OBJECT_H
#define _ML_SAFE_WALL_OBJECT_H

#include "GameObject.hpp"
#include "MetatextureView.hpp"

class SafeWallView: public MetatextureView
{
public:
    SafeWallView(TileMaterialManager &matman);

};

class SafeWallObject: public GameObject
{
public:
    SafeWallObject(Level *level);

public:
    void setup_view(TileMaterialManager &matman) override;

};

#endif
