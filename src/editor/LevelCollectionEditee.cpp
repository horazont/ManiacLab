/**********************************************************************
File name: LevelCollectionEditee.cpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "LevelCollectionEditee.hpp"

/* LevelCollectionEditee */

LevelCollectionEditee::LevelCollectionEditee(
        const SharedLevelCollection &editee,
        const std::string &name):
    _name(name),
    _editee(editee),
    _level_map(),
    _changed(),
    _level_changed(),
    _level_created(),
    _level_deleted()
{
    for (auto &level: editee->levels) {
        _level_map[level->get_uuid()] = level;
    }
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

void LevelCollectionEditee::require_valid_level_uuid(
    const PyEngine::UUID &uuid)
{
    auto it = _level_map.find(uuid);
    if (it != _level_map.end()) {
        throw std::invalid_argument(
            "Duplicate level uuid: "+uuid.to_string());
    }
}

std::vector<SharedLevel> &LevelCollectionEditee::levels()
{
    return _editee->levels;
}

SharedLevel LevelCollectionEditee::add_level(const SharedLevel &level)
{
    require_valid_level_uuid(level->get_uuid());

    _editee->levels.push_back(level);
    _level_map[level->get_uuid()] = level;
    level_created(level);

    return level;
}

void LevelCollectionEditee::delete_level(const SharedLevel &level)
{
    std::vector<SharedLevel> &levels = _editee->levels;
    auto it = std::find(levels.begin(), levels.end(), level);
    if (it != levels.end()) {
        level_deleted(level);
        levels.erase(it);
        _level_map.erase(level->get_uuid());
    } else {
        throw std::logic_error("Attempt to delete foreign level.");
    }
}

SharedLevel LevelCollectionEditee::new_level(const PyEngine::UUID &uuid)
{
    require_valid_level_uuid(uuid);
    LevelData *new_level = new LevelData();
    new_level->set_uuid(uuid);
    new_level->set_display_name(uuid.to_string());
    return add_level(SharedLevel(new_level));
}

void LevelCollectionEditee::set_name(const std::string &name)
{
    _name = name;
}
