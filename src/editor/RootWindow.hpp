/**********************************************************************
File name: RootWindow.hpp
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
#ifndef _ML_ROOT_WINDOW_H
#define _ML_ROOT_WINDOW_H

#include <unordered_map>
#include <vector>

#include <gtkmm.h>

#include "TilesetEditor.hpp"
#include "TilesetEditDetails.hpp"

class RootWindow: public Gtk::Window
{
public:
    RootWindow(BaseObjectType *cobject,
               const Glib::RefPtr<Gtk::Builder> &builder);
    virtual ~RootWindow();

private:
    std::unordered_map<std::string, SharedTileset> _tileset_by_name;
    std::unordered_map<TilesetData*, TilesetEditee*> _loaded_tilesets;
    std::unordered_map<TilesetData*, TilesetEditor*> _tileset_editors;
    std::vector<Editor*> _editors;

    Editor *_current_editor;

private:
    Glib::RefPtr<Gtk::RecentManager> _recent;
    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::MenuItem *_menu_level;
    Gtk::MenuItem *_menu_tileset;
    Glib::RefPtr<Gtk::ActionGroup> _actions_level;
    Glib::RefPtr<Gtk::ActionGroup> _actions_tileset;
    Gtk::Notebook *_tabs;

    Glib::RefPtr<Gtk::Action> _action_save, _action_save_as;

    Gtk::Dialog *_dlg_create_tileset;
    Gtk::FileChooserDialog *_dlg_open_file;
    Gtk::FileChooserDialog *_dlg_save_file;
    TilesetEditDetails *_dlg_tileset_details;

protected:
    void action_file_new_tileset();
    void action_file_open();
    void action_file_save();
    void action_file_save_as();
    void action_file_quit();
    void action_tileset_edit_details();
    void dlg_create_tileset_activate();
    void dlg_create_tileset_response(int response_id);
    void dlg_open_file_activate();
    void dlg_open_file_response(int response_id);
    void dlg_save_file_activate();
    void dlg_save_file_response(int response_id);
    void tabs_switch_page(Gtk::Widget *page, guint page_num);

public:
    void open_file(const std::string &filename);
    void save_file(const std::string &filename);
    void switch_editor(Editor *new_editor);

public:
    const Glib::RefPtr<Gtk::Builder> &get_builder() const;
    Gtk::MenuItem *get_menu_level();
    Gtk::MenuItem *get_menu_tileset();

    inline Glib::RefPtr<Gtk::ActionGroup> get_actions_level() {
        return _actions_level;
    };

    inline Glib::RefPtr<Gtk::ActionGroup> get_actions_tileset() {
        return _actions_tileset;
    };

public:
    /**
     * Acquire a tileset editor for the given tileset editee. If there
     * is no editor for the given editee, a new one will be created.
     */
    TilesetEditor *get_tileset_editor(TilesetEditee *editee);

    /**
     * Load a tileset if it is not loaded already and return an editee
     * for the tileset.
     */
    TilesetEditee *make_tileset_editable(const SharedTileset &data);

    TilesetEditor *get_tileset_editor_by_name(const std::string &unique_name);
    TilesetEditee *get_tileset_editee_by_name(const std::string &unique_name);
    const SharedTileset &get_tileset_by_name(const std::string &unique_name);

};

#endif