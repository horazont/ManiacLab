/**********************************************************************
File name: TilesetEditor.hpp
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
#ifndef _ML_TILESET_EDITOR_H
#define _ML_TILESET_EDITOR_H

#include "Editor.hpp"

#include "io/TilesetData.hpp"

#include "TilesetEditee.hpp"

class TileListModelColumns: public Gtk::TreeModelColumnRecord
{
public:
    TileListModelColumns();

    Gtk::TreeModelColumn<SharedTile> col_tile;
    Gtk::TreeModelColumn<Glib::ustring> col_display_name;
};

class TilesetEditor: public Editor
{
public:
    TilesetEditor(RootWindow *root,
                  Gtk::Container *parent,
                  TilesetEditee *editee);

private:
    TilesetEditee *_editee;
    sigc::connection _conn_delete_tile;
    sigc::connection _conn_edit_details;
    sigc::connection _conn_new_tile;

    Gtk::Grid _tile_prop_grid;
    Gtk::Switch _tile_blocking,
                _tile_destructible,
                _tile_edible,
                _tile_gravity_affected,
                _tile_rollable,
                _tile_sticky;
    Gtk::SpinButton _tile_roll_radius,
                    _tile_temp_coefficient;
    Gtk::Entry _tile_display_name,
               _tile_unique_name;

    TileListModelColumns _tile_columns;
    Glib::RefPtr<Gtk::ListStore> _tile_list;
    Gtk::TreeView _tile_list_view;

private:
    SharedTile _current_tile;

private:
    void bind_editing_done(Gtk::Entry &widget,
                           void (TilesetEditor::*handler)());
    void bind_editing_done(Gtk::Switch &widget,
                           void (TilesetEditor::*handler)());
    void configure_entry(Gtk::Entry &widget);
    void configure_spin_button(Gtk::SpinButton &widget);
    void configure_switch(Gtk::Switch &widget);
    void initialize_contents();
    void setup_tile_list_view(Gtk::TreeView &view);
    void setup_tile_property_grid(Gtk::Grid &grid);
    void setup_tile_toolbar(Gtk::Toolbar &toolbar);

protected:
    Gtk::TreeNodeChildren::iterator find_tile_row(const SharedTile &tile);
    void flush_tile_props();
    void select_tile(const SharedTile &tile);
    void update_tile_props();

protected:
    void action_new_tile();
    void action_delete_tile();
    void action_edit_details();
    void editee_tile_changed(
        TilesetEditee *editee,
        const SharedTile &tile);
    void editee_tile_created(
        TilesetEditee *editee,
        const SharedTile &tile);
    void editee_tile_deleted(
        TilesetEditee *editee,
        const SharedTile &tile);
    void tile_list_view_row_activated(
        const Gtk::TreeModel::Path &path,
        Gtk::TreeViewColumn *column);

public:
    inline TilesetEditee *editee() {
        return _editee;
    };

public:
    void disable() override;
    void enable() override;

public:
    void file_save(const PyEngine::StreamHandle &stream) override;

};

#endif
