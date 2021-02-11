#ifndef ML_ROCK_OBJECT_H
#define ML_ROCK_OBJECT_H

#include "game_object.hpp"

class RockObject: public GameObject
{
public:
    explicit RockObject(Level &level);
};

#endif
