#ifndef _ML_ROCK_OBJECT_H
#define _ML_ROCK_OBJECT_H

#include "GameObject.hpp"
#include "MetatextureView.hpp"

class RockView: public MetatextureView
{
public:
    RockView(TileMaterialManager &matman);

};

class RockObject: public GameObject
{
public:
    RockObject(Level *level);

protected:
    std::unique_ptr<ObjectView> setup_view(TileMaterialManager &matman) override;

};

#endif
