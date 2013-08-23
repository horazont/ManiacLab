/**********************************************************************
File name: DuplicateTile.cpp
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
#include "DuplicateTile.hpp"

#include "GTKUtils.hpp"

using namespace Glib;
using namespace Gtk;

/* DuplicateTile */

DuplicateTile::DuplicateTile(
        BaseObjectType *cobject,
        const RefPtr<Builder> &builder):
    UniqueNameDialog(
        cobject,
        builder,
        get_entry(builder, "duplicate_tile_unique_name")),
    _rewrite_to_new(nullptr),
    _response(false)
{
    _builder->get_widget("duplicate_tile_rewrite_to_new",
                         _rewrite_to_new);

    show_all_children();
}

void DuplicateTile::response_abort()
{
    _response = false;
}

void DuplicateTile::response_ok()
{
    _response = true;
}

bool DuplicateTile::get_duplicate_settings(
        std::string &new_name,
        bool &rewrite_references_to_self)
{
    _response = false;
    _unique_name->set_text(new_name);
    _rewrite_to_new->set_active(rewrite_references_to_self);
    run();
    new_name = _unique_name->get_text();
    rewrite_references_to_self = _rewrite_to_new->get_active();
    return _response;
}
