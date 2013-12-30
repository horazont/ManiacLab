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
