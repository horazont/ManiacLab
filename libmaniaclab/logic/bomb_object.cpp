#include "bomb_object.hpp"

#include <iostream>

static const CellStamp bomb_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

const ObjectInfo BombObject::INFO(
    true,
    true,
    false,
    true,
    true,
    true,
    false,
    0.5,
    heat_capacity_metal,
    bomb_object_stamp);


/* BombObject */

BombObject::BombObject(Level &level):
    GameObject(BombObject::INFO, level)
{

}

void BombObject::headache(GameObject*)
{
    explode();
}

void BombObject::explode()
{
    level.add_large_explosion(cell.x, cell.y, 1, 1);
    destruct_self();
}

void BombObject::explosion_touch()
{
    explode();
}

bool BombObject::impact(GameObject*)
{
    explode();
    return false;
}

void BombObject::update()
{
    /*const SimFloat temperature =
            level->measure_object_avg(cell.x, cell.y, [this](const LabCell &cell){std::cout << cell.heat_energy << std::endl; return cell.heat_energy / info.temp_coefficient;});*/
    /*uintptr_t len = 0;
    const CoordPair *coords =
            info.stamp.get_map_coords(&len);*/
    const SimFloat temperature = level.measure_object_avg(
                *this,
                [this](const LabCell &cell){return cell.heat_energy / info.temp_coefficient;});
    /*const SimFloat pressure =
            level->measure_border_avg(cell.x, cell.y, [](const LabCell &cell){return cell.air_pressure;}, true);*/
    // std::cout << "bomb temperature = " << temperature << std::endl; // << "; pressure = " << pressure << std::endl;

    if (temperature > temperature_threshold) {
        explode();
        return;
    }
    GameObject::update();
}

const SimFloat BombObject::temperature_threshold = 390;
