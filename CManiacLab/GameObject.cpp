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

#include "IOUtils.hpp"
#include "Errors.hpp"
#include "Physics.hpp"

Template::Template():
    stamp(0),
    is_gravity_affected(false),
    is_rollable(false),
    temp_coefficient(1.0),
    radius(0.5)
{

}

Template::Template(Template const &ref):
    stamp(ref.stamp),
    is_gravity_affected(ref.is_gravity_affected),
    is_rollable(ref.is_rollable),
    temp_coefficient(ref.temp_coefficient),
    radius(ref.radius)
{

}

Template& Template::operator=(Template const &ref)
{
    if (stamp) {
	delete stamp;
    }
    stamp = ref.stamp;
    is_gravity_affected = ref.is_gravity_affected;
    is_rollable = ref.is_rollable;
    temp_coefficient = ref.temp_coefficient;
    return *this;
}

Template::~Template()
{
    if (stamp) {
	delete stamp;
    }
}

void Template::load_version_1(PyEngine::StreamHandle &instream)
{
    uint16_t binflags = instream->readUInt16();

    assert(binflags & TBF_HAS_STAMP);
    if (stamp) {
	delete stamp;
    }
    BoolCellStamp boolstamp;
    load_cell_stamp(instream, boolstamp);
    stamp = new Stamp(boolstamp);

    is_gravity_affected = (binflags & TBF_GRAVITY_AFFECTED) != 0;
    is_rollable = (binflags & TBF_ROLLABLE) != 0;

    temp_coefficient = instream->readT<double>();
    radius = instream->readT<double>();
}

void Template::save_version_1(PyEngine::StreamHandle &outstream)
{
    uint16_t binflags = 0;
    if (is_gravity_affected) {
	binflags |= TBF_GRAVITY_AFFECTED;
    }
    if (is_rollable) {
	binflags |= TBF_ROLLABLE;
    }
    if (stamp) {
	binflags |= TBF_HAS_STAMP;
	save_cell_stamp(outstream, stamp->get_map());
    }

    outstream->writeT<double>(temp_coefficient);
    outstream->writeT<double>(radius);
}

void Template::load_bin(PyEngine::StreamHandle &instream)
{
    uint8_t version = instream->readUInt8();
    switch (version)
    {
    case 1:
	load_version_1(instream);
	break;
    default:
	throw PyEngine::Exception("invalid template format version");
    }
}

void Template::save_bin(PyEngine::StreamHandle &outstream)
{
    outstream->writeUInt8(1);
    save_version_1(outstream);
}


GameObject::GameObject():
    Template::Template(),
    x(0), y(0), phi(0),
    movement(0)
{

}

GameObject::GameObject(GameObject const &ref):
    Template::Template(ref),
    x(ref.x), y(ref.y), phi(ref.phi),
    movement(ref.movement)
{

}

GameObject& GameObject::operator=(GameObject const &ref)
{
    Template::operator=(static_cast<Template const&>(ref));
    x = ref.x;
    y = ref.y;
    phi = ref.phi;
    movement = nullptr;
    phy = ref.phy;
    return *this;
}

GameObject::~GameObject()
{

}
