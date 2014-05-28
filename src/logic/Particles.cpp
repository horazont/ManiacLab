#include "Particles.hpp"

#include <CEngine/Math/Vectors.hpp>

#include "Level.hpp"

inline void update_coord(PyEngine::TimeFloat deltaT,
                         float &v, float &vv, float &av)
{
    v = v + vv*deltaT + av*deltaT/2;
    vv = vv + av*deltaT;
}

inline void handle_collision(
    Automaton &physics,
    Cell &current_cell,
    float &x, float &vx,
    float &y, float &vy)
{
    PyEngine::Vector2f posstep(-vx, -vy);
    posstep.normalize();
    PyEngine::Vector2f pos(x, y);
    pos *= subdivision_count;
    Cell *cell = &current_cell, *prev_cell = &current_cell;
    CellMetadata *meta = nullptr;
    for (unsigned int step = 0; step < 10; step++) {
        pos += posstep;
        const CoordInt cx = round(pos[PyEngine::eX]);
        const CoordInt cy = round(pos[PyEngine::eY]);

        prev_cell = cell;
        cell = physics.safe_cell_at(cx, cy);
        if (!cell) {
            break;
        }

        meta = physics.meta_at(cx, cy);

        if (meta->blocked) {
            continue;
        }

        break;
    }

    x = pos[PyEngine::eX] / subdivision_count;
    y = pos[PyEngine::eY] / subdivision_count;

    if (!cell) {
        // out of game
        return;
    } else {
        if (!meta->blocked) {
            cell = prev_cell;
        }
        // found end of object

        posstep *= -1;

        const PyEngine::Vector2f incoming_ray(vx, vy);
        PyEngine::Vector2f new_v =
            incoming_ray - (2*incoming_ray * posstep) * posstep;
        const float vmag = new_v.length();
        vx = new_v[PyEngine::eX] * 0.4;
        vx = vx + (((float)random() / RAND_MAX) * 2.0 - 1.0)*vmag*0.3;
        vy = new_v[PyEngine::eY] * 0.4;
        vy = vy + (((float)random() / RAND_MAX) * 2.0 - 1.0)*vmag*0.3;
        return;
    }

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
        update_coord(deltaT, part->phi, part->vphi, part->aphi);

        switch (part->type) {
        case ParticleType::FIRE:
        {
            const uint32_t old_ctr = part->ctr;
            const uint32_t new_ctr = part->age * 25;
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
                subpart->ay = -0.2;
                subpart->phi = ((float)random() / RAND_MAX)*2*3.14159;
                subpart->vphi = part->vphi;
                subpart->aphi = 0;
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

        switch (part->type) {
        case ParticleType::FIRE:
        {
            // const double prev_vx = part->vx;
            // const double prev_vy = part->vy;
            // cell->flow[0] = cell->flow[0] * config.flow_damping - prev_vy * (1.0 - config.flow_damping);
            // cell->flow[1] = cell->flow[1] * config.flow_damping - prev_vx * (1.0 - config.flow_damping);
            part->vx = part->vx * 0.999 - cell->flow[1] * 0.001;
            part->vy = part->vy * 0.999 - cell->flow[0] * 0.001;

            cell->heat_energy += FIRE_PARTICLE_TEMPERATURE_RISE * (
                meta->blocked ?
                meta->obj->info.temp_coefficient :
                cell->air_pressure);

            if (meta->blocked) {
                meta->obj->ignition_touch();
            }
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

        if (meta->blocked) {
            handle_collision(
                physics,
                *cell,
                part->x,
                part->vx,
                part->y,
                part->vy);
            continue;
        }

    }


}
