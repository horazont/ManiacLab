#include "Weapon.hpp"

/* Flamethrower */

Flamethrower::Flamethrower():
    fuel(10000),
    subticks(0)
{

}

bool Flamethrower::empty() const
{
    return fuel > 0;
}

void Flamethrower::fire(Level *level,
                        const CoordPair &user,
                        const CoordPair &direction)
{
    if (fuel == 0) {
        return;
    }

    subticks += 1;
    if (subticks < 13) {
        return;
    }
    subticks = 0;

    fuel -= 1;

    PhysicsParticle *const part = level->particles().spawn();
    part->type = ParticleType::FIRE;
    part->x = user.x + 0.5 + direction.x * 0.6;
    part->y = user.y + 0.5 + direction.y * 0.6;
    part->vx = 8. * direction.x + ((float)random() / RAND_MAX)*0.6 - 0.3;
    part->vy = 8. * direction.y + ((float)random() / RAND_MAX)*0.6 - 0.3;
    part->ax = 0;
    part->ay = 0;
    part->phi = ((float)random() / RAND_MAX)*2*3.14159;
    part->vphi = (((float)random() / RAND_MAX)-0.5)*3.14159/5.0;
    part->aphi = 0;
    part->lifetime = 1;
}
