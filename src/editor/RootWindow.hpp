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
#include <deque>

#include <gtkmm.h>

#include <CEngine/VFS/FileSystem.hpp>
#include <CEngine/VFS/Mount.hpp>

#include "TilesetEditor.hpp"
#include "TilesetEditDetails.hpp"
#include "NewTile.hpp"
#include "DuplicateTile.hpp"
#include "Operation.hpp"
#include "OpenImage.hpp"
#include "VFSFileChooserDialog.hpp"

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

    std::deque<std::unique_ptr<Operation>> _undo_stack;
    std::deque<std::unique_ptr<Operation>> _redo_stack;

    PyEngine::FileSystem _vfs;

private:
    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::MenuItem *_menu_level;
    Gtk::MenuItem *_menu_tileset;
    Glib::RefPtr<Gtk::ActionGroup> _actions_level;
    Glib::RefPtr<Gtk::ActionGroup> _actions_tileset;
    Glib::RefPtr<Gtk::AccelGroup> _accel_level;
    Glib::RefPtr<Gtk::AccelGroup> _accel_tileset;
    Glib::RefPtr<Gtk::Action> _action_undo;
    Glib::RefPtr<Gtk::Action> _action_redo;
    Gtk::Notebook *_tabs;

    Glib::RefPtr<Gtk::Action> _action_save, _action_save_as;

    Gtk::AboutDialog *_dlg_about;
    DuplicateTile *_dlg_duplicate_tile;
    NewTile *_dlg_new_tile;
    OpenImage *_dlg_open_image;
    TilesetEditDetails *_dlg_tileset_details;
    VFSFileChooserDialog _dlg_open_vfs_file;
    VFSFileChooserDialog _dlg_save_vfs_file;

protected:
    void action_file_new_tileset();
    void action_file_open();
    void action_file_save();
    void action_file_save_as();
    void action_file_quit();
    void action_edit_undo();
    void action_edit_redo();
    void action_help_about();
    void tabs_switch_page(Gtk::Widget *page, guint page_num);

protected:
    void update_undo_redo_sensitivity();

public:
    void open_file(const std::string &filename);
    void process_args(int argc, char *argv[]);
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

    inline TilesetEditDetails *get_dlg_tileset_details() {
        return _dlg_tileset_details;
    };

    inline NewTile *get_dlg_new_tile() {
        return _dlg_new_tile;
    };

    inline OpenImage *get_dlg_open_image() {
        return _dlg_open_image;
    };

    inline DuplicateTile *get_dlg_duplicate_tile() {
        return _dlg_duplicate_tile;
    };

public:
    void disable_level_controls();
    void disable_tileset_controls();
    void enable_level_controls();
    void enable_tileset_controls();

public:
    void execute_operation(OperationPtr &&operation);

public:
    void close_editor(
        Editor *editor);

    void close_tileset(
        TilesetEditee *editee);

    /**
     * Acquire a tileset editor for the given tileset editee. If there
     * is no editor for the given editee, a new one will be created.
     */
    TilesetEditor *get_tileset_editor(TilesetEditee *editee);

    /**
     * Load a tileset if it is not loaded already and return an editee
     * for the tileset.
     */
    TilesetEditee *make_tileset_editable(
        const SharedTileset &data,
        const std::string &name);

    void rename_editor(Editor *editor, const std::string &new_name);

    /**
     * Renames a tileset which is currently being edited. If the new
     * name conflicts with the name of another tileset, the other
     * tileset will be renamed to a null name.
     */
    void rename_tileset(
        TilesetEditee *editee,
        const std::string &new_name);

    void update_tab_name(Editor *editor);

    TilesetEditor *get_tileset_editor_by_name(const std::string &unique_name);
    TilesetEditee *get_tileset_editee_by_name(const std::string &unique_name);
    const SharedTileset &get_tileset_by_name(const std::string &unique_name);
};

#endif
