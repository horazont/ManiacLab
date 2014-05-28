#ifndef _ML_PLAYER_OBJECT_H
#define _ML_PLAYER_OBJECT_H

#include "GameObject.hpp"
#include "MetatextureView.hpp"
#include "Weapon.hpp"

enum Action {
    ACTION_NONE,
    ACTION_MOVE,
    ACTION_FIRE_WEAPON
};

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
    Action action;
    MoveDirection move_direction;
    Weapon *active_weapon;
    Flamethrower flamethrower;

protected:
    std::unique_ptr<ObjectView> setup_view(
        TileMaterialManager &matman) override;

public:
    bool idle() override;

};

#endif
