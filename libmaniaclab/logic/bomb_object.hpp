#ifndef _ML_BOMB_OBJECT_H
#define _ML_BOMB_OBJECT_H

#include "game_object.hpp"


struct BombObject: public GameObject
{
public:
    static const ObjectInfo INFO;

public:
    explicit BombObject(Level &level);

public:
    void headache(GameObject *from_object) override;
    void explode();
    void explosion_touch() override;
    bool impact(GameObject *on_object) override;
    void update() override;

public:
    static const SimFloat temperature_threshold;

};

#endif
