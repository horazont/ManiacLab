#include "LevelCollectionEditee.hpp"

/* LevelCollectionEditee */

LevelCollectionEditee::LevelCollectionEditee(
        const SharedLevelCollection &editee,
        const std::string &name):
    _name(name),
    _editee(editee),
    _changed(),
    _level_changed(),
    _level_created(),
    _level_deleted()
{

}

void LevelCollectionEditee::changed()
{
    _changed(this);
}

void LevelCollectionEditee::level_changed(const SharedLevel &level)
{
    _level_changed(this, level);
    changed();
}

void LevelCollectionEditee::level_created(const SharedLevel &level)
{
    _level_created(this, level);
    changed();
}

void LevelCollectionEditee::level_deleted(const SharedLevel &level)
{
    _level_deleted(this, level);
    changed();
}

std::vector<SharedLevel> &LevelCollectionEditee::levels()
{
    return _editee->levels;
}

void LevelCollectionEditee::set_name(const std::string &name)
{
    _name = name;
}
