/**********************************************************************
File name: NewTile.cpp
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
#include "NewTile.hpp"

#include "GTKUtils.hpp"

using namespace Glib;
using namespace Gtk;

/* NewTile */

NewTile::NewTile(
        BaseObjectType *cobject,
        const RefPtr<Builder> &builder):
    UniqueNameDialog(
        cobject,
        builder,
        get_entry(builder, "new_tile_unique_name"))
{
    show_all_children();
}

void NewTile::response_abort()
{
    _unique_name->set_text("");
}

std::string NewTile::get_unique_name()
{
    _unique_name->set_text("");
    run();
    return _unique_name->get_text();
}

