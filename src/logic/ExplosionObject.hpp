#ifndef _ML_EXPLOSION_OBJECT_H
#define _ML_EXPLOSION_OBJECT_H

#include "GameObject.hpp"

struct ExplosionView: public ObjectView
{
public:
    ExplosionView();

};

struct ExplosionObject: public GameObject
{
public:
    ExplosionObject(Level *level);

private:
    TickCounter die_at;

public:
    void update() override;
    void setup_view();
    void setup_view(TileMaterialManager &matman) override;

};

#endif
