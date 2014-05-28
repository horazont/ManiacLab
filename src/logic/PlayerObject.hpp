#ifndef _ML_PLAYER_OBJECT_H
#define _ML_PLAYER_OBJECT_H

#include "GameObject.hpp"
#include "MetatextureView.hpp"

struct PlayerView: public MetatextureView
{
public:
    PlayerView(TileMaterialManager &matman);

};

struct PlayerObject: public GameObject
{
public:
    PlayerObject(Level *level);

public:
    MoveDirection acting;

protected:
    std::unique_ptr<ObjectView> setup_view(
        TileMaterialManager &matman) override;

public:
    bool idle() override;

};

#endif
