/**********************************************************************
File name: UniqueNameDialog.hpp
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
#ifndef _ML_UNIQUE_NAME_DIALOG_H
#define _ML_UNIQUE_NAME_DIALOG_H

#include <gtkmm.h>

class UniqueNameDialog: public Gtk::Dialog
{
public:
    typedef sigc::signal<bool, const std::string&> CheckSignal;

public:
    UniqueNameDialog(
        BaseObjectType *cobject,
        const Glib::RefPtr<Gtk::Builder> &builder,
        Gtk::Entry *unique_name);

protected:
    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::Entry *_unique_name;
    CheckSignal _check_name_signal;

protected:
    virtual bool check_unique_name(const std::string &name);
    static Gtk::Entry *get_entry(
        const Glib::RefPtr<Gtk::Builder> &builder,
        const std::string &name);
    void on_response(int response_id) override;
    virtual void response_abort();
    virtual void response_ok();
    void unique_name_activate();
    void unique_name_changed();

public:
    inline CheckSignal &signal_check_name() {
        return _check_name_signal;
    };

};

#endif
