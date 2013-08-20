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
    Dialog(cobject),
    _builder(builder)
{
    _builder->get_widget("new_tile_unique_name", _unique_name);

    _unique_name->signal_activate().connect(
        sigc::mem_fun(*this, &NewTile::unique_name_activate));
    _unique_name->signal_changed().connect(
        sigc::mem_fun(*this, &NewTile::unique_name_changed));

    show_all_children();
}

void NewTile::on_response(int response_id)
{
    switch (response_id) {
    case 2:
    {
        std::string unique_name = _unique_name->get_text();

        if (unique_name == "") {
            message_dlg(*this,
                "Invalid unique name",
                "The unique name must not be empty.",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }

        hide();
        break;
    }
    case 1:
    default:
        _unique_name->set_text("");
        hide();
        return;
    };
}

void NewTile::unique_name_activate()
{
    on_response(2);
}

void NewTile::unique_name_changed()
{
    if (_unique_name->get_text() == "") {
        _unique_name->set_icon_from_stock(StockID("gtk-dialog-warning"));
    } else {
        _unique_name->unset_icon();
    }
}

std::string NewTile::get_unique_name()
{
    _unique_name->set_text("");
    run();
    return _unique_name->get_text();
}

