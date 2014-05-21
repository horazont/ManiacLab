/**********************************************************************
File name: LevelCollectionEditor.hpp
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
#ifndef _LEVEL_COLLECTION_EDITOR_H
#define _LEVEL_COLLECTION_EDITOR_H

#include "Editor.hpp"

#include "LevelCollectionEditee.hpp"
#include "LevelEditor.hpp"

class LevelCollectionEditor: public Editor
{
protected:
    class LevelListModelColumns: public Gtk::TreeModelColumnRecord
    {
    public:
        LevelListModelColumns();

        Gtk::TreeModelColumn<SharedLevel> col_level;
        Gtk::TreeModelColumn<Glib::ustring> col_uuid;
        Gtk::TreeModelColumn<Glib::ustring> col_display_name;

    };

public:
    LevelCollectionEditor(
        RootWindow *root,
        Gtk::Container *parent,
        LevelCollectionEditee *editee);
    virtual ~LevelCollectionEditor();

private:
    LevelCollectionEditee *_editee;
    sigc::connection _conn_delete_level;
    sigc::connection _conn_move_level_down;
    sigc::connection _conn_move_level_up;
    sigc::connection _conn_new_level;

    LevelEditor _level_editor;

    LevelListModelColumns _level_columns;
    Glib::RefPtr<Gtk::ListStore> _level_list;
    Gtk::TreeView _level_list_view;

    Gtk::TreeView _tile_tree_view;

    SharedLevel _current_level;

protected:
    Gtk::TreeNodeChildren::iterator find_level_row(
        const SharedLevel &level);
    SharedLevel get_selected_level();
    void select_level(const SharedLevel &level);

public:
    const std::string &get_name() const override;
    std::string get_tab_name() const override;
    std::string get_vfs_dirname() const override;
    void set_name(const std::string &name) override;

public:
    inline LevelCollectionEditee *editee() const {
        return _editee;
    };

public:
    void action_delete_level();
    void action_move_level_down();
    void action_move_level_up();
    void action_new_level();
    void editee_level_created(
        LevelCollectionEditee *editee,
        const SharedLevel &level);
    void editee_level_deleted(
        LevelCollectionEditee *editee,
        const SharedLevel &level);
    void level_list_view_row_activated(
        const Gtk::TreeModel::Path &path,
        Gtk::TreeViewColumn *column);
    void tile_tree_view_row_activated(
        const Gtk::TreeModel::Path &path,
        Gtk::TreeViewColumn *column);

public:
    void disable() override;
    void enable() override;
    void file_save(const PyEngine::StreamHandle &stream) override;

};

#endif
