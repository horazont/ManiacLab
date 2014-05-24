/**********************************************************************
File name: GameObject.cpp
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
#include "GameObject.hpp"

#include "Errors.hpp"
#include "Physics.hpp"

/* FrameState */

void FrameState::reset()
{
    explode = false;
    ignite = false;
    own_temperature = NAN;
    surr_temperature = NAN;
}

/* ObjectInfo */

ObjectInfo::ObjectInfo(const CellStamp &stamp):
    TileData(),
    stamp(stamp)
{
    TileData::stamp = stamp;
}

ObjectInfo::ObjectInfo(const TileData &src):
    TileData(src),
    stamp(src.stamp)
{

}

/* ObjectView */

ObjectView::ObjectView():
    invalidated(true)
{

}

/* GameObject */

GameObject::GameObject(const ObjectInfo &info):
    frame_state(),
    info(info),
    x(0),
    y(0),
    phi(0),
    phy(),
    view()
{

}
