/**********************************************************************
File name: OpenImage.cpp
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
#include "OpenImage.hpp"

using namespace Glib;
using namespace Gtk;

OpenImage::OpenImage(BaseObjectType *cobject,
                     const RefPtr<Builder> &builder):
    FileChooserDialog(cobject),
    _response_ok(false),
    _filename()
{
    signal_file_activated().connect(
        sigc::mem_fun(*this, &OpenImage::on_file_activated));
}

void OpenImage::on_file_activated()
{
    on_response(2);
}

void OpenImage::on_response(int response_id)
{
    switch (response_id)
    {
    case 2:
    {
        _response_ok = true;
        _filename = get_filename();
        hide();
        return;
    }
    case 1:
    default:
        _response_ok = false;
        hide();
        break;
    }
}

RefPtr<Gdk::Pixbuf> OpenImage::select_image(bool auto_add_to_recent)
{
    _response_ok = false;
    run();
    if (!_response_ok) {
        return RefPtr<Gdk::Pixbuf>();
    }

    if (auto_add_to_recent) {
        RecentManager::get_default()->add_item(get_uri());
    }
    return Gdk::Pixbuf::create_from_file(_filename);
}
