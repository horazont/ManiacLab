/**********************************************************************
File name: OpenImage.hpp
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
#ifndef _ML_OPEN_IMAGE_H
#define _ML_OPEN_IMAGE_H

#include <gtkmm.h>

class OpenImage: public Gtk::FileChooserDialog
{
public:
    OpenImage(BaseObjectType *cobject,
               const Glib::RefPtr<Gtk::Builder> &builder);

private:
    bool _response_ok;
    std::string _filename;

protected:
    void on_file_activated();
    void on_response(int response_id) override;

public:
    Glib::RefPtr<Gdk::Pixbuf> select_image(
        bool auto_add_to_recent = true);

};

#endif
