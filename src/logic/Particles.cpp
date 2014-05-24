#include "Particles.hpp"

inline void update_coord(PyEngine::TimeFloat deltaT,
                         float &v, float &vv, float &av)
{
    v = v + vv*deltaT + av*deltaT/2;
    vv = vv + av*deltaT;
}

inline void update_one(PhysicsParticle *part,
                       PyEngine::TimeFloat deltaT)
{
    part->age += deltaT;
    if (part->age > part->lifetime) {
        part->alive = false;
        return;
    }
    update_coord(deltaT, part->x, part->vx, part->ax);
    update_coord(deltaT, part->y, part->vy, part->ay);
}

/* ParticleSystem */

ParticleSystem::Particle *ParticleSystem::spawn()
{
    Particle *part = allocate();
    _active.push_back(part);
    return part;
}

void ParticleSystem::spawn_generator(size_t n, const Generator &generator)
{
    for (size_t i = 0; i < n; i++) {
        Particle *const part = allocate();
        generator(part);
        if (part->lifetime > 0) {
            _active.push_back(part);
        } else {
            release(part);
        }
    }
}

void ParticleSystem::update(PyEngine::TimeFloat deltaT)
{
    for (auto it = _active.begin();
         it != _active.end();
         it++)
    {
        Particle *const part = *it;
        update_one(part, deltaT);
        if (!part->alive) {
            it = _active.erase(it);
        }
    }
}
