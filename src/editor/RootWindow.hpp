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
#include "DuplicateTile.hpp"
#include "Operation.hpp"
#include "OpenImage.hpp"
#include "VFSFileChooserDialog.hpp"
#include "LevelCollectionEditor.hpp"
#include "LevelCollectionEditee.hpp"

class TileTreeModelColumns: public Gtk::TreeModelColumnRecord
{
public:
    TileTreeModelColumns();

    Gtk::TreeModelColumn<SharedTileset> col_tileset;
    Gtk::TreeModelColumn<SharedTile> col_tile;
    Gtk::TreeModelColumn<Glib::ustring> col_display_name;
};

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
    std::unordered_map<std::string, SharedLevelCollection> _level_collection_by_name;
    std::unordered_map<LevelCollection*, LevelCollectionEditee*> _loaded_level_collections;
    std::unordered_map<LevelCollection*, LevelCollectionEditor*> _level_collection_editors;
    std::unordered_map<TileData*,
                       Cairo::RefPtr<Cairo::ImageSurface>> _tile_pictures;
    std::vector<Editor*> _editors;

    Editor *_current_editor;

    std::deque<std::unique_ptr<Operation>> _undo_stack;
    std::deque<std::unique_ptr<Operation>> _redo_stack;

    PyEngine::FileSystem _vfs;

private:
    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::MenuItem *_menu_level;
    Gtk::MenuItem *_menu_level_collection;
    Gtk::MenuItem *_menu_tileset;
    Glib::RefPtr<Gtk::ActionGroup> _actions_level;
    Glib::RefPtr<Gtk::ActionGroup> _actions_level_collection;
    Glib::RefPtr<Gtk::ActionGroup> _actions_tileset;
    Glib::RefPtr<Gtk::AccelGroup> _accel_level;
    Glib::RefPtr<Gtk::AccelGroup> _accel_tileset;
    Glib::RefPtr<Gtk::Action> _action_undo;
    Glib::RefPtr<Gtk::Action> _action_redo;
    Gtk::Notebook *_tabs;

    Glib::RefPtr<Gtk::Action> _action_file_save;
    Glib::RefPtr<Gtk::Action> _action_file_save_as;
    Glib::RefPtr<Gtk::Action> _action_file_close;

    Gtk::AboutDialog *_dlg_about;
    DuplicateTile *_dlg_duplicate_tile;
    OpenImage *_dlg_open_image;
    TilesetEditDetails *_dlg_tileset_details;
    VFSFileChooserDialog _dlg_open_vfs_file;
    VFSFileChooserDialog _dlg_save_vfs_file;

    TileTreeModelColumns _tile_columns;
    Glib::RefPtr<Gtk::TreeStore> _tile_tree;

protected:
    void action_file_new_tileset();
    void action_file_new_level();
    void action_file_open();
    void action_file_save();
    void action_file_save_as();
    void action_file_close();
    void action_file_quit();
    void action_edit_undo();
    void action_edit_redo();
    void action_help_about();
    void tabs_switch_page(Gtk::Widget *page, guint page_num);

protected:
    void add_tile_node(const Gtk::TreeNodeChildren::iterator &parent,
                       const SharedTileset &tileset,
                       const SharedTile &tile);
    void any_tile_changed(TilesetEditee *editee,
                          const SharedTile &tile);
    void any_tile_created(TilesetEditee *editee,
                          const SharedTile &tile);
    void any_tile_deleted(TilesetEditee *editee,
                          const SharedTile &tile);
    void delete_editor(Editor *editor);
    Gtk::TreeNodeChildren::iterator find_tile_row(
        const SharedTile &tile);
    Gtk::TreeNodeChildren::iterator find_tileset_row(
        const SharedTileset &tileset);
    static Gtk::Box *new_editor_box();
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

    inline OpenImage *get_dlg_open_image() {
        return _dlg_open_image;
    };

    inline DuplicateTile *get_dlg_duplicate_tile() {
        return _dlg_duplicate_tile;
    };

    inline TileTreeModelColumns &get_tile_columns() {
        return _tile_columns;
    };

    inline Glib::RefPtr<Gtk::TreeStore> get_tile_tree() {
        return _tile_tree;
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

    void close_level_collection(
        LevelCollectionEditee *editee);

    void close_tileset(
        TilesetEditee *editee);

    void flush_picture_cache();
    void flush_picture_cache(const SharedTile &tile);

    LevelCollectionEditor *get_level_collection_editor(
        LevelCollectionEditee *editee);

    Cairo::RefPtr<Cairo::ImageSurface> get_tile_picture(const SharedTile &tile);

    /**
     * Acquire a tileset editor for the given tileset editee. If there
     * is no editor for the given editee, a new one will be created.
     */
    TilesetEditor *get_tileset_editor(TilesetEditee *editee);

    std::string get_tileset_name(const SharedTileset &tileset);

    TilesetEditee *load_tileset_by_name(
        const std::string &name);

    /**
     * Lookup a tile from a tileset and return shared pointers to
     * both the tileset and the tile. If the tileset is not loaded
     * yet, it will be loaded into background (no tabs will be
     * created). If the tileset does not exist, nullptr is returned
     * for both pointers. If the tileset does exist, but the tile is
     * not in the tileset, nullptr is returned for the second pointer,
     * but the first points to the tileset. Otherwise, valid pointers
     * are contained in both fields.
     *
     * @param tileset_name unique name of the tileset
     * @param uuid UUID of the tile to find
     * @return two pointers, one for the tileset and one for the tile
     * which was returned by the lookup (see above for details of the
     * error cases).
     */
    LevelData::TileBinding lookup_tile(
        const std::string &tileset_name,
        const PyEngine::UUID &uuid);

    LevelCollectionEditee *make_level_collection_editable(
        const SharedLevelCollection &data,
        const std::string &name);

    /**
     * Load a tileset if it is not loaded already and return an editee
     * for the tileset.
     */
    TilesetEditee *make_tileset_editable(
        const SharedTileset &data,
        const std::string &name);

    void rename_editor(Editor *editor, const std::string &new_name);

    void rename_level_collection(
        LevelCollectionEditee *editee,
        const std::string &new_name);

    /**
     * Renames a tileset which is currently being edited. If the new
     * name conflicts with the name of another tileset, the other
     * tileset will be renamed to a null name.
     */
    void rename_tileset(
        TilesetEditee *editee,
        const std::string &new_name);

    SharedTileset require_tileset(
        const std::string &tileset_name);

    void update_tab_name(Editor *editor);

    TilesetEditor *get_tileset_editor_by_name(const std::string &unique_name);
    TilesetEditee *get_tileset_editee_by_name(const std::string &unique_name);
    const SharedTileset &get_tileset_by_name(const std::string &unique_name);
};

#endif
