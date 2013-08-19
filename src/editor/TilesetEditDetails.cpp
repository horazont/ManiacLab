/**********************************************************************
File name: TilesetEditDetails.cpp
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
#include "TilesetEditDetails.hpp"

#include "GTKUtils.hpp"

using namespace Glib;
using namespace Gtk;

/* TilesetEditDetails */

TilesetEditDetails::TilesetEditDetails(
        BaseObjectType *cobject,
        const RefPtr<Builder> &builder):
    Dialog(cobject),
    _builder(builder),
    _editee(nullptr)
{
    _builder->get_widget("tileset_details_author", _author);
    _builder->get_widget("tileset_details_description", _description);
    _builder->get_widget("tileset_details_display_name", _display_name);
    _builder->get_widget("tileset_details_license", _license);
    _builder->get_widget("tileset_details_unique_name", _unique_name);
    _builder->get_widget("tileset_details_version", _version);

    signal_response().connect(
        sigc::mem_fun(*this, &TilesetEditDetails::response));

    _display_name->signal_changed().connect(
        sigc::mem_fun(*this, &TilesetEditDetails::display_name_changed));

    show_all_children();
}

void TilesetEditDetails::display_name_changed()
{
    if (_display_name->get_text() == "") {
        _display_name->set_icon_from_stock(StockID("gtk-dialog-warning"));
    } else {
        _display_name->unset_icon();
    }
}

void TilesetEditDetails::response(int response_id)
{
    switch (response_id) {
    case 2:
    {
        std::string display_name = _display_name->get_text();

        if (display_name == "") {
            message_dlg(*this,
                "Display name is empty",
                "As the display name is used in the user interface, it "
                "must not be empty to ensure proper user experience.",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }

        hide();

        _editee->header.display_name = display_name;
        _editee->header.author = _author->get_text();
        _editee->header.license = _license->get_text();
        _editee->header.version = _version->get_text();
        _editee->header.description =
            _description->get_buffer()->get_text();

        break;
    }
    case 1:
    default:
        hide();
        break;
    }
}

void TilesetEditDetails::edit_tileset(const SharedTileset &editee)
{
    _editee = editee;
    _author->set_text(editee->header.author);
    _description->get_buffer()->set_text(editee->header.description);
    _display_name->set_text(editee->header.display_name);
    _license->set_text(editee->header.license);
    _unique_name->set_text(editee->header.unique_name);
    _version->set_text(editee->header.version);
    run();
}
