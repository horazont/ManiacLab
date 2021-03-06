#ifndef _ML_BOMB_OBJECT_H
#define _ML_BOMB_OBJECT_H

#include "GameObject.hpp"
#include "MetatextureView.hpp"

struct BombView: public MetatextureView
{
public:
    BombView(TileMaterialManager &matman);

};

struct BombObject: public GameObject
{
public:
    BombObject(Level *level);

protected:
    std::unique_ptr<ObjectView> setup_view(
        TileMaterialManager &matman) override;

public:
    void headache(GameObject *from_object) override;
    void explode();
    void explosion_touch() override;
    bool impact(GameObject *on_object) override;

};

#endif
