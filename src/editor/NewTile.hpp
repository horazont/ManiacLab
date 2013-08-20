/**********************************************************************
File name: NewTile.hpp
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
#ifndef _ML_NEW_TILE_H
#define _ML_NEW_TILE_H

#include <gtkmm.h>

class NewTile: public Gtk::Dialog
{
public:
    NewTile(BaseObjectType *cobject,
            const Glib::RefPtr<Gtk::Builder> &builder);

private:
    Glib::RefPtr<Gtk::Builder> _builder;

private:
    Gtk::Entry *_unique_name;

protected:
    void on_response(int response_id) override;
    void unique_name_activate();
    void unique_name_changed();

public:
    std::string get_unique_name();

};

#endif
