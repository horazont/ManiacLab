#ifndef _ML_WEAPON_H
#define _ML_WEAPON_H

#include "GameObject.hpp"

struct Weapon: public ViewableObject
{
public:
    virtual bool empty() const = 0;

    virtual void fire(
        Level *level,
        const CoordPair &user,
        const CoordPair &direction) = 0;

};

struct Flamethrower: public Weapon
{
public:
    Flamethrower();

private:
    TickCounter fuel;
    TickCounter subticks;

public:
    bool empty() const override;

    void fire(Level *level,
              const CoordPair &user,
              const CoordPair &direction) override;


};

#endif
