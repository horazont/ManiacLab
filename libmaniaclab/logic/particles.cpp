#include "particles.hpp"

#include <ffengine/math/vector.hpp>

#include "logic/level.hpp"

inline void update_coord(ffe::TimeInterval deltaT,
                         float &v, float &vv, float &av)
{
    v = v + vv*deltaT + av*deltaT/2;
    vv = vv + av*deltaT;
}

inline void handle_collision(
    NativeLabSim &physics,
    LabCell &current_cell,
    float &x, float &vx,
    float &y, float &vy)
{
    Vector2f posstep(-vx, -vy);
    posstep.normalize();
    Vector2f pos(x, y);
    pos *= subdivision_count;
    const LabCell *cell = &current_cell, *prev_cell = &current_cell;
    LabCellMeta *meta = nullptr;
    for (unsigned int step = 0; step < 10; step++) {
        pos += posstep;
        const CoordInt cx = round(pos[eX]);
        const CoordInt cy = round(pos[eY]);

        prev_cell = cell;
        cell = physics.safe_front_cell_at(cx, cy);
        if (!cell) {
            break;
        }

        meta = &physics.meta_at(cx, cy);

        if (meta->blocked) {
            continue;
        }

        break;
    }

    x = pos[eX] / subdivision_count;
    y = pos[eY] / subdivision_count;

    if (!cell) {
        // out of game
        return;
    } else {
        if (!meta->blocked) {
            cell = prev_cell;
        }
        // found end of object

        posstep *= -1;

        const Vector2f incoming_ray(vx, vy);
        Vector2f new_v =
            incoming_ray - (2*incoming_ray * posstep) * posstep;
        const float vmag = new_v.length();
        vx = new_v[eX] * 0.4f;
        vx = vx + (((float)random() / RAND_MAX) * 2.0f - 1.0f)*vmag*0.3f;
        vy = new_v[eY] * 0.4f;
        vy = vy + (((float)random() / RAND_MAX) * 2.0f - 1.0f)*vmag*0.3f;
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
        generator(i, part);
        if (part->lifetime > 0) {
            _active.push_back(part);
        } else {
            release(part);
        }
    }
}

void ParticleSystem::update(ffe::TimeInterval deltaT)
{
    NativeLabSim &physics = _level.physics();
    // const SimulationConfig &config = physics.config();

    static constexpr SimFloat fire_primary_cell_flow_influence = SimFloat(1e-1);
    static constexpr SimFloat inv_fire_primary_cell_flow_influence = SimFloat(1)-fire_primary_cell_flow_influence;
    static constexpr SimFloat fire_secondary_cell_flow_influence = SimFloat(5e-1);
    static constexpr SimFloat inv_fire_secondary_cell_flow_influence = SimFloat(1)-fire_secondary_cell_flow_influence;

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
            const uint32_t new_ctr = part->age * 25.f;
            part->ctr = new_ctr;

            const uint32_t to_spawn = new_ctr - old_ctr;

            for (uint32_t i = 0; i < to_spawn; i++)
            {
                Particle *const subpart = spawn();
                subpart->type = ParticleType::FIRE_SECONDARY;
                subpart->lifetime = 4+((float)rand() / RAND_MAX)*2.f-1.f;
                subpart->x = part->x - ((float)rand() / RAND_MAX) * part->vx * 0.01f;
                subpart->y = part->y - ((float)rand() / RAND_MAX) * part->vy * 0.01f;
                subpart->vx = part->vx * 0.1f;
                subpart->vy = part->vy * 0.1f;
                subpart->ax = 0.f;
                subpart->ay = -0.2f;
                subpart->phi = ((float)random() / RAND_MAX)*2.f*M_PIf32;
                subpart->vphi = part->vphi;
                subpart->aphi = 0.f;
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
            || phy.x >= _level.width()*subdivision_count
            || phy.y >= _level.height()*subdivision_count)
        {
            continue;
        }
        LabCell &cell = physics.writable_cell_at(phy.x, phy.y);
        LabCellMeta &meta = physics.meta_at(phy.x, phy.y);

        switch (part->type) {
        case ParticleType::FIRE:
        {
            // const double prev_vx = part->vx;
            // const double prev_vy = part->vy;
            // cell->flow[0] = cell->flow[0] * config.flow_damping - prev_vy * (1.0 - config.flow_damping);
            // cell->flow[1] = cell->flow[1] * config.flow_damping - prev_vx * (1.0 - config.flow_damping);
            if (!meta.blocked) {
                part->vx = part->vx * inv_fire_primary_cell_flow_influence - cell.flow[eX] * fire_primary_cell_flow_influence;
                part->vy = part->vy * inv_fire_primary_cell_flow_influence - cell.flow[eY] * fire_primary_cell_flow_influence;
            }

            cell.heat_energy += FIRE_PARTICLE_TEMPERATURE_RISE * cell.heat_capacity_cache;

            if (meta.blocked) {
                meta.obj->ignition_touch();
            }
            break;
        }
        case ParticleType::FIRE_SECONDARY:
        {
            if (!meta.blocked) {
                part->vx = part->vx * inv_fire_secondary_cell_flow_influence - cell.flow[eX] * fire_secondary_cell_flow_influence;
                part->vy = part->vy * inv_fire_secondary_cell_flow_influence - cell.flow[eY] * fire_secondary_cell_flow_influence;
            }

            // cell->fog += 0.001;
            break;
        }
        }

        if (meta.blocked) {
            handle_collision(
                physics,
                cell,
                part->x,
                part->vx,
                part->y,
                part->vy);
            continue;
        }

    }


}
