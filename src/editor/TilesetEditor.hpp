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
#include "TileEditor.hpp"
#include "Operation.hpp"

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
    sigc::connection _conn_duplicate_tile;
    sigc::connection _conn_edit_details;
    sigc::connection _conn_new_tile;
    sigc::connection _conn_tile_changed;
    sigc::connection _conn_import_visual;

    bool _updating;
    Gtk::VBox _tile_props;
    Glib::RefPtr<Gtk::SizeGroup> _tile_size_group;
    Gtk::Switch _tile_actor,
                _tile_blocking,
                _tile_destructible,
                _tile_edible,
                _tile_gravity_affected,
                _tile_movable,
                _tile_rollable,
                _tile_sticky;
    Gtk::SpinButton _tile_roll_radius,
                    _tile_temp_coefficient;
    Gtk::Entry _tile_display_name,
               _tile_unique_name;

    TileListModelColumns _tile_columns;
    Glib::RefPtr<Gtk::ListStore> _tile_list;
    Gtk::TreeView _tile_list_view;
    TileEditor _tile_editor;

private:
    SharedTile _current_tile;

private:
    void bind_editing_done(Gtk::Entry &widget,
                           void (TilesetEditor::*handler)());
    void bind_editing_done(Gtk::Switch &widget,
                           void (TilesetEditor::*handler)());
    void configure_entry(Gtk::Entry &widget);
    void configure_entry(
        Gtk::Entry &widget,
        const std::function<void(const std::string&)> &updater);
    void configure_spin_button(
        Gtk::SpinButton &widget,
        const std::function<void(float)> &updater);
    void configure_switch(
        Gtk::Switch &widget,
        const std::function<void(bool)> &updater);
    void execute_operation(OperationPtr &&operation);
    void frame_grid(Gtk::Grid &grid, Gtk::Box &parent,
                    const Glib::ustring &label_text);
    void initialize_contents();
    void setup_tile_list_view(Gtk::TreeView &view);
    void setup_tile_props_generic(
        Gtk::Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group);
    void setup_tile_props_game_play(
        Gtk::Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group);
    void setup_tile_props_physics(
        Gtk::Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group);
    void setup_tile_props_actor(
        Gtk::Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group);
    void setup_tile_toolbar(Gtk::Toolbar &toolbar);

protected:
    Gtk::TreeNodeChildren::iterator find_tile_row(const SharedTile &tile);
    void flush_tile_props();
    SharedTile get_selected_tile();
    void select_tile(const SharedTile &tile);
    template <typename value_t, typename operation_t,
              typename std::remove_reference<typename std::remove_const<value_t>::type>::type TileData::*value_ptr>
    void update_tile_prop(
        value_t value)
    {
        TileData *tilep = _current_tile.get();
        if (tilep->*value_ptr == value) {
            return;
        }
        execute_operation(OperationPtr(new operation_t(
            _editee, _current_tile, value)));
    }
    void update_tile_props();

protected:
    void action_new_tile();
    void action_duplicate_tile();
    void action_delete_tile();
    void action_edit_details();
    void action_import_visual();
    void editee_tile_changed(
        TilesetEditee *editee,
        const SharedTile &tile);
    void editee_tile_created(
        TilesetEditee *editee,
        const SharedTile &tile);
    void editee_tile_deleted(
        TilesetEditee *editee,
        const SharedTile &tile);
    void switch_button_button_press(
        GdkEventButton *event,
        Gtk::Switch *widget);
    void tile_list_view_row_activated(
        const Gtk::TreeModel::Path &path,
        Gtk::TreeViewColumn *column);

public:
    inline TilesetEditee *editee() {
        return _editee;
    };

public:
    const std::string &get_name() const override;
    std::string get_tab_name() const override;
    void set_name(const std::string &name) override;

public:
    void disable() override;
    void enable() override;

public:
    void file_save(const PyEngine::StreamHandle &stream) override;

};

#endif
