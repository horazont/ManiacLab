/**********************************************************************
File name: Editor.hpp
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
#ifndef _ML_EDITOR_H
#define _ML_EDITOR_H

#include <gtkmm.h>

#include <CEngine/IO/Stream.hpp>

class RootWindow;

class Editor
{
public:
    Editor(RootWindow *root, Gtk::Container *parent);
    Editor(const Editor &ref) = delete;
    Editor& operator=(const Editor &ref) = delete;
    virtual ~Editor();

protected:
    RootWindow *_root;
    Gtk::Container *_parent;
    bool _filename_known;
    std::string _filename;

public:
    inline bool get_filename_known() const {
        return _filename_known;
    };

    inline const std::string &get_filename() const {
        return _filename;
    };

    inline Gtk::Container *get_parent() const {
        return _parent;
    };

    void set_filename(const std::string &name);

    void unset_filename();

public:
    virtual const std::string &get_name() const = 0;
    virtual std::string get_tab_name() const = 0;
    virtual void set_name(const std::string &name) = 0;

public:
    virtual void disable();
    virtual void enable();

public:
    virtual void edit_cut();
    virtual void edit_copy();
    virtual void edit_paste();
    virtual void edit_delete();
    virtual void edit_select_all();
    virtual void file_save(const PyEngine::StreamHandle &stream);

};

#endif
