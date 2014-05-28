#include "WallObject.hpp"

static const CellStamp squarewall_object_stamp(
    {
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
    }
);

static const CellStamp roundwall_object_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

static ObjectInfo safewallsq_object_info(
    true,
    false,
    false,
    false,
    false,
    false,
    true,
    0.0,
    1.0,
    squarewall_object_stamp);

static ObjectInfo safewallrd_object_info(
    true,
    false,
    false,
    false,
    false,
    true,
    true,
    0.5,
    1.0,
    roundwall_object_stamp);

/* WallView */

std::unique_ptr<ObjectView> wall_view(
    TileMaterialManager &matman,
    Level &level,
    const CoordPair &cell,
    const std::string &prefix)
{
    bool neigh_top = false;
    bool neigh_left = false;
    bool neigh_right = false;
    bool neigh_bottom = false;

    if (cell.x > 0) {
        LevelCell *const neigh = level.get_cell(cell.x-1, cell.y);
        neigh_left = neigh->here && dynamic_cast<WallObject*>(neigh->here);
    }

    if (cell.y > 0) {
        LevelCell *const neigh = level.get_cell(cell.x, cell.y-1);
        neigh_top = neigh->here && dynamic_cast<WallObject*>(neigh->here);
    }

    if (cell.x < level.get_width() - 1)
    {
        LevelCell *const neigh = level.get_cell(cell.x+1, cell.y);
        neigh_right = neigh->here && dynamic_cast<WallObject*>(neigh->here);
    }

    if (cell.y < level.get_height() - 1)
    {
        LevelCell *const neigh = level.get_cell(cell.x, cell.y+1);
        neigh_bottom = neigh->here && dynamic_cast<WallObject*>(neigh->here);
    }

    uint_least8_t entry = (neigh_left << 3)
        | (neigh_top << 2)
        | (neigh_right << 1)
        | neigh_bottom;

    static const char map[16] = {
        '0', // 0b0000
        '1', // 0b0001
        '1', // 0b0010
        '3', // 0b0011
        '1', // 0b0100
        '5', // 0b0101
        '3', // 0b0110
        '7', // 0b0111
        '1', // 0b1000
        '3', // 0b1001
        '5', // 0b1010
        '7', // 0b1011
        '3', // 0b1100
        '7', // 0b1101
        '7', // 0b1110
        'f', // 0b1111
    };

    static const unsigned int phi_offset[16] = {
        0, // 0b0000
        0, // 0b0001
        3, // 0b0010
        0, // 0b0011
        2, // 0b0100
        0, // 0b0101
        3, // 0b0110
        0, // 0b0111
        1, // 0b1000
        1, // 0b1001
        3, // 0b1010
        1, // 0b1011
        2, // 0b1100
        2, // 0b1101
        3, // 0b1110
        0, // 0b1111
    };

    return std::unique_ptr<PhiOffsetMetatextureView>(
        new PhiOffsetMetatextureView(
            matman.require_material(
                prefix + map[entry]),
            phi_offset[entry] * 3.14159 / 2));
}

/* WallObject */

WallObject::WallObject(ObjectInfo &info, Level *level):
    GameObject(info, level)
{
}

/* SafeWallObject */

SafeWallObject::SafeWallObject(Level *level):
    WallObject(safewallsq_object_info, level)
{

}

void SafeWallObject::setup_view(TileMaterialManager &matman)
{
    view = wall_view(matman,
                     *level,
                     cell,
                     "safewallsq");
}
