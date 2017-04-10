#ifndef _ML_BOMB_OBJECT_H
#define _ML_BOMB_OBJECT_H

#include "game_object.hpp"


struct BombObject: public GameObject
{
public:
    BombObject(Level *level);

public:
    void headache(GameObject *from_object) override;
    void explode();
    void explosion_touch() override;
    bool impact(GameObject *on_object) override;

};

#endif
