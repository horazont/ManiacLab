/**********************************************************************
File name: GTKUtils.hpp
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
#ifndef _ML_GTK_UTILS_H
#define _ML_GTK_UTILS_H

#include <gtkmm.h>

#include "io/TilesetData.hpp"

sigc::connection bind_action(
    const Glib::RefPtr<Gtk::Builder> &builder,
    const std::string &name,
    const Gtk::Action::SlotActivate &slot,
    Glib::RefPtr<Gtk::Action> *action_dest = nullptr);

int message_dlg(Gtk::Window &parent,
                const std::string &primary_text,
                const std::string &secondary_text,
                Gtk::MessageType message_type,
                Gtk::ButtonsType buttons);

Glib::RefPtr<Gdk::Pixbuf> tile_image_data_to_pixbuf(ImageData *data);
bool pixbuf_to_tile_image_data(
    const Glib::RefPtr<Gdk::Pixbuf> &src, ImageData *data);
Cairo::RefPtr<Cairo::Surface>
    get_temporary_cairo_surface_for_tile_image_data(ImageData *data);


#endif
