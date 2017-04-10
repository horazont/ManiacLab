#ifndef _ML_SAFE_WALL_OBJECT_H
#define _ML_SAFE_WALL_OBJECT_H

#include "logic/game_object.hpp"



class WallObject: public GameObject
{
public:
    WallObject(ObjectInfo &info, Level *level);

};

class SafeWallObject: public WallObject
{
public:
    SafeWallObject(Level *level);

};


#endif
