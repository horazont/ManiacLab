#include "Particles.hpp"

#include "Level.hpp"

inline void update_coord(PyEngine::TimeFloat deltaT,
                         float &v, float &vv, float &av)
{
    v = v + vv*deltaT + av*deltaT/2;
    vv = vv + av*deltaT;
}

inline void handle_collision(
    Automaton &physics,
    float &x, float &vx,
    float &y, float &vy)
{

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
    part->alive = true;
    part->age = 0;
    part->ctr = 0;
    _active.push_back(part);
    return part;
}

void ParticleSystem::spawn_generator(size_t n, const Generator &generator)
{
    for (size_t i = 0; i < n; i++) {
        Particle *const part = allocate();
        part->alive = true;
        part->age = 0;
        part->ctr = 0;
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

        switch (part->type) {
        case ParticleType::FIRE:
        {
            const uint32_t old_ctr = part->ctr;
            const uint32_t new_ctr = part->age * 50;
            part->ctr = new_ctr;

            const uint32_t to_spawn = new_ctr - old_ctr;

            for (uint32_t i = 0; i < to_spawn; i++)
            {
                Particle *const subpart = spawn();
                subpart->type = ParticleType::FIRE_SECONDARY;
                subpart->lifetime = 4+((float)rand() / RAND_MAX)*2-1;
                subpart->x = part->x - ((float)rand() / RAND_MAX) * part->vx * 0.01;
                subpart->y = part->y - ((float)rand() / RAND_MAX) * part->vy * 0.01;
                subpart->vx = part->vx * 0.1;
                subpart->vy = part->vy * 0.1;
                subpart->ax = 0;
                subpart->ay = 0;
            }

            break;
        }
        case ParticleType::FIRE_SECONDARY:
        {
            break;
        }
        }

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
            handle_collision(
                physics,
                part->x, part->y,
                part->vx, part->vy);
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
        case ParticleType::FIRE_SECONDARY:
        {
            part->vx = part->vx * 0.995 - cell->flow[1] * 0.005;
            part->vy = part->vy * 0.995 - cell->flow[0] * 0.005;

            // cell->fog += 0.001;
            break;
        }
        }

    }


}
