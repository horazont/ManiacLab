#include "Particles.hpp"

#include "Level.hpp"

inline void update_coord(PyEngine::TimeFloat deltaT,
                         float &v, float &vv, float &av)
{
    v = v + vv*deltaT + av*deltaT/2;
    vv = vv + av*deltaT;
}

/* ParticleSystem */

ParticleSystem::ParticleSystem(Level &level):
    GenericParticleSystem<PhysicsParticle, 1024>(),
    _level(level)
{

}

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
    Automaton &physics = _level.physics();
    // const SimulationConfig &config = physics.config();

    for (auto it = _active.begin();
         it != _active.end();
         it++)
    {
        Particle *const part = *it;
        part->age += deltaT;
        if (part->age > part->lifetime) {
            it = _active.erase(it);
            continue;
        }
        update_coord(deltaT, part->x, part->vx, part->ax);
        update_coord(deltaT, part->y, part->vy, part->ay);

        CoordPair phy = _level.get_physics_coords(part->x, part->y);
        if (phy.x < 0 || phy.y < 0
            || phy.x >= _level.get_width()*subdivision_count
            || phy.y >= _level.get_height()*subdivision_count)
        {
            continue;
        }
        Cell *cell = physics.cell_at(phy.x, phy.y);
        CellMetadata *meta = physics.meta_at(phy.x, phy.y);
        if (meta->blocked) {
            // FIXME: collision!
            continue;
        }

        switch (part->type) {
        case ParticleType::FIRE:
        {
            // const double prev_vx = part->vx;
            // const double prev_vy = part->vy;
            // cell->flow[0] = cell->flow[0] * config.flow_damping - prev_vy * (1.0 - config.flow_damping);
            // cell->flow[1] = cell->flow[1] * config.flow_damping - prev_vx * (1.0 - config.flow_damping);
            part->vx = part->vx * 0.999 - cell->flow[1] * 0.001;
            part->vy = part->vy * 0.999 - cell->flow[0] * 0.001;

            cell->heat_energy += 0.1 * cell->air_pressure;
            break;
        }
        }
    }
}
