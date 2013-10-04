/**********************************************************************
File name: TilesetEditor.cpp
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
#include "TilesetEditor.hpp"

#include "RootWindow.hpp"
#include "GTKUtils.hpp"
#include "TilesetOperations.hpp"

using namespace Gtk;
using namespace Glib;

inline Label *managed_label(
    const std::string &text,
    RefPtr<SizeGroup> group = RefPtr<SizeGroup>())
{
    Label *label = manage(new Label());
    label->set_text(text);
    label->set_alignment(ALIGN_START, ALIGN_CENTER);
    //label->set_single_line_mode(false);
    //label->set_line_wrap_mode(Pango::WRAP_WORD);
    //label->set_ellipsize(Pango::ELLIPSIZE_MIDDLE);
    if (group) {
        group->add_widget(*label);
    }
    return label;
}

inline void add_label(const std::string &text, Gtk::Container *parent)
{
    parent->add(*managed_label(text));
}

/* TileListModelColumns */

TileListModelColumns::TileListModelColumns():
    TreeModelColumnRecord(),
    col_tile(),
    col_display_name()
{
    add(col_tile);
    add(col_display_name);
}

/* TilesetEditor */

TilesetEditor::TilesetEditor(
        RootWindow *root,
        Container *parent,
        TilesetEditee *editee):
    Editor(root, parent),
    _editee(editee),
    _conn_delete_tile(),
    _conn_duplicate_tile(),
    _conn_edit_details(),
    _conn_new_tile(),
    _updating(false),
    _tile_props(),
    _tile_size_group(SizeGroup::create(SIZE_GROUP_HORIZONTAL)),
    _tile_actor(),
    _tile_blocking(),
    _tile_destructible(),
    _tile_edible(),
    _tile_gravity_affected(),
    _tile_rollable(),
    _tile_sticky(),
    _tile_roll_radius(0, 2),
    _tile_temp_coefficient(0, 2),
    _tile_display_name(),
    _tile_unique_name(),
    _tile_columns(),
    _tile_list(ListStore::create(_tile_columns)),
    _tile_list_view(_tile_list),
    _tile_editor(root, editee),
    _current_tile(nullptr)
{
    _conn_tile_changed = editee->signal_tile_changed().connect(
        sigc::mem_fun(*this, &TilesetEditor::editee_tile_changed));
    editee->signal_tile_created().connect(
        sigc::mem_fun(*this, &TilesetEditor::editee_tile_created));
    editee->signal_tile_deleted().connect(
        sigc::mem_fun(*this, &TilesetEditor::editee_tile_deleted));

    _tile_unique_name.set_icon_from_stock(StockID("gtk-dialog-info"));
    _tile_unique_name.set_icon_tooltip_text("The unique name cannot be changed.");
    _tile_unique_name.set_icon_activatable(false);
    _tile_unique_name.set_sensitive(false);

    configure_switch(
        _tile_actor,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileActor,
                                             &TileData::is_actor>,
            this,
            std::placeholders::_1));
    configure_switch(
        _tile_blocking,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileBlocking,
                                             &TileData::is_blocking>,
            this,
            std::placeholders::_1));
    configure_switch(
        _tile_destructible,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileDestructible,
                                             &TileData::is_destructible>,
            this,
            std::placeholders::_1));
    configure_switch(
        _tile_edible,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileEdible,
                                             &TileData::is_edible>,
            this,
            std::placeholders::_1));
    configure_switch(
        _tile_gravity_affected,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileGravityAffected,
                                             &TileData::is_gravity_affected>,
            this,
            std::placeholders::_1));
    configure_switch(
        _tile_movable,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileMovable,
                                             &TileData::is_movable>,
            this,
            std::placeholders::_1));
    configure_switch(
        _tile_rollable,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileRollable,
                                             &TileData::is_rollable>,
            this,
            std::placeholders::_1));
    configure_switch(
        _tile_sticky,
        std::bind(
            &TilesetEditor::update_tile_prop<bool,
                                             OpSetTileSticky,
                                             &TileData::is_sticky>,
            this,
            std::placeholders::_1));

    configure_spin_button(
        _tile_roll_radius,
        std::bind(
            &TilesetEditor::update_tile_prop<float,
                                             OpSetTileRollRadius,
                                             &TileData::roll_radius>,
            this,
            std::placeholders::_1));
    _tile_roll_radius.set_increments(0.01, 0.1);
    _tile_roll_radius.set_range(0, 1);

    configure_spin_button(
        _tile_temp_coefficient,
        std::bind(
            &TilesetEditor::update_tile_prop<float,
                                             OpSetTileTempCoefficient,
                                             &TileData::temp_coefficient>,
            this,
            std::placeholders::_1));
    _tile_temp_coefficient.set_increments(0.01, 0.1);
    _tile_temp_coefficient.set_range(1e-2, 1e3);

    configure_entry(
        _tile_display_name,
        std::bind(
            &TilesetEditor::update_tile_prop<const std::string&,
                                             OpSetTileDisplayName,
                                             &TileData::display_name>,
            this,
            std::placeholders::_1));
    configure_entry(_tile_unique_name);

    Gtk::Grid *tile_props_generic = manage(new Grid());
    setup_tile_props_generic(*tile_props_generic, _tile_size_group);
    frame_grid(*tile_props_generic, _tile_props, "Generic properties");

    Gtk::Grid *tile_props_game_play = manage(new Grid());
    setup_tile_props_game_play(*tile_props_game_play, _tile_size_group);
    frame_grid(*tile_props_game_play, _tile_props, "Game play");

    Gtk::Grid *tile_props_physics = manage(new Grid());
    setup_tile_props_physics(*tile_props_physics, _tile_size_group);
    frame_grid(*tile_props_physics, _tile_props, "Physics");

    Gtk::Grid *tile_props_actor = manage(new Grid());
    setup_tile_props_actor(*tile_props_actor, _tile_size_group);
    frame_grid(*tile_props_actor, _tile_props, "Actor");

    ScrolledWindow *tile_prop_scroll = manage(new ScrolledWindow());
    Viewport *tile_prop_viewport = manage(new Viewport(
        tile_prop_scroll->get_hadjustment(),
        tile_prop_scroll->get_vadjustment()));
    tile_prop_scroll->add(*tile_prop_viewport);
    tile_prop_viewport->add(_tile_props);

    setup_tile_list_view(_tile_list_view);
    _tile_list_view.signal_row_activated().connect(
        sigc::mem_fun(*this, &TilesetEditor::tile_list_view_row_activated));

    Toolbar *tile_toolbar = manage(new Toolbar());
    setup_tile_toolbar(*tile_toolbar);

    ScrolledWindow *tile_list_view_scroll = manage(new ScrolledWindow());
    tile_list_view_scroll->add(_tile_list_view);

    VBox *tiles_box = manage(new VBox());
    tiles_box->pack_start(*tile_list_view_scroll, true, true);
    tiles_box->pack_end(*tile_toolbar, false, false);

    VPaned *left_panel = manage(new VPaned());
    left_panel->pack1(*tiles_box, false, true);
    left_panel->pack2(*tile_prop_scroll, false, true);
    left_panel->set_position(300);

    _tile_editor.set_valign(ALIGN_CENTER);
    _tile_editor.set_halign(ALIGN_CENTER);

    HPaned *main_panel = manage(new HPaned());
    main_panel->set_hexpand(true);
    main_panel->add(*left_panel);
    main_panel->add(_tile_editor);
    main_panel->set_position(200);

    parent->add(*main_panel);
    parent->show_all_children();

    _tile_props.set_sensitive(false);

    initialize_contents();
}

void TilesetEditor::bind_editing_done(
        Entry &widget,
        void (TilesetEditor::*handler)())
{
    widget.signal_focus_out_event().connect(
        [this, handler](GdkEventFocus *ev){
            (this->*handler)();
            return false;
        });
}

void TilesetEditor::bind_editing_done(
        Switch &widget,
        void (TilesetEditor::*handler)())
{
    widget.property_active().signal_changed().connect(
        sigc::mem_fun(*this, handler));
}

void TilesetEditor::configure_entry(Entry &widget)
{
    widget.set_hexpand(true);
    widget.set_halign(ALIGN_FILL);
    widget.set_width_chars(12);
}

void TilesetEditor::configure_entry(
    Entry &widget,
    const std::function<void(const std::string&)> &updater)
{
    configure_entry(widget);
    Entry *widget_ptr = &widget;
    widget.signal_focus_out_event().connect(
        [this, widget_ptr, updater](GdkEventFocus *event) {
            if (_updating) {
                return false;
            }
            updater(widget_ptr->get_text());
            return false;
        });
}

void TilesetEditor::configure_spin_button(
    SpinButton &widget,
    const std::function<void(float)> &updater)
{
    widget.set_hexpand(true);
    widget.set_halign(ALIGN_FILL);
    SpinButton *widget_ptr = &widget;
    widget.signal_focus_out_event().connect(
        [this, widget_ptr, updater](GdkEventFocus *event) {
            if (_updating) {
                return false;
            }
            updater(widget_ptr->get_value());
            return false;
        });
}

void TilesetEditor::configure_switch(
    Switch &widget,
    const std::function<void(bool)> &updater)
{
    widget.set_hexpand(false);
    widget.set_halign(ALIGN_START);
    widget.set_can_focus(true);
    Switch *widget_ptr = &widget;
    widget.signal_event_after().connect(
        [this, widget_ptr](GdkEvent *event) mutable {
            if (event->type != GDK_BUTTON_PRESS) {
                return;
            }
            this->switch_button_button_press(&event->button, widget_ptr);
        });
    widget.property_active().signal_changed().connect(
        [this, widget_ptr, updater]() mutable {
            if (_updating) {
                return;
            }
            updater(widget_ptr->get_active());
        });
}

void TilesetEditor::execute_operation(OperationPtr &&operation)
{
    _root->execute_operation(std::move(operation));
}

void TilesetEditor::frame_grid(Gtk::Grid &grid, Gtk::Box &parent,
                               const ustring &label_text)
{
    Frame *frame = manage(new Frame());
    frame->set_shadow_type(SHADOW_NONE);
    Label *label = manage(new Label());
    frame->set_label_widget(*label);
    label->set_markup("<b>"+label_text+"</b>");
    parent.pack_start(*frame, false, true);

    Alignment *align = manage(new Alignment());
    frame->add(*align);
    align->set_padding(0, 0, 12, 0);

    align->add(grid);
}

void TilesetEditor::initialize_contents()
{
    for (auto &tile: _editee->tiles()) {
        editee_tile_created(_editee, tile);
    }
}

void TilesetEditor::setup_tile_list_view(Gtk::TreeView &view)
{
    _tile_list->set_sort_column(
        _tile_columns.col_display_name,
        SORT_ASCENDING
        );
    view.set_vexpand(false);
    view.set_model(_tile_list);
    view.append_column(
        "Tile name", _tile_columns.col_display_name);
}

void TilesetEditor::setup_tile_props_generic(
    Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group)
{
    int top = 0;

    grid.attach(*managed_label("Unique name", group), 0, top, 1, 1);
    grid.attach(_tile_unique_name, 1, top++, 1, 1);

    grid.attach(*managed_label("Display name", group), 0, top, 1, 1);
    grid.attach(_tile_display_name, 1, top++, 1, 1);
}

void TilesetEditor::setup_tile_props_game_play(
    Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group)
{
    int top = 0;

    grid.attach(*managed_label("Edible", group), 0, top, 1, 1);
    grid.attach(_tile_edible, 1, top++, 1, 1);

    grid.attach(*managed_label("Movable", group), 0, top, 1, 1);
    grid.attach(_tile_movable, 1, top++, 1, 1);

    grid.attach(*managed_label("Roll radius", group), 0, top, 1, 1);
    grid.attach(_tile_roll_radius, 1, top++, 1, 1);
}

void TilesetEditor::setup_tile_props_physics(
    Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group)
{
    int top = 0;

    grid.attach(*managed_label("Blocking", group), 0, top, 1, 1);
    grid.attach(_tile_blocking, 1, top++, 1, 1);

    grid.attach(*managed_label("Destructible", group), 0, top, 1, 1);
    grid.attach(_tile_destructible, 1, top++, 1, 1);

    grid.attach(*managed_label("Gravity affected", group), 0, top, 1, 1);
    grid.attach(_tile_gravity_affected, 1, top++, 1, 1);

    grid.attach(*managed_label("Round (rollable)", group), 0, top, 1, 1);
    grid.attach(_tile_rollable, 1, top++, 1, 1);

    grid.attach(*managed_label("Sticky", group), 0, top, 1, 1);
    grid.attach(_tile_sticky, 1, top++, 1, 1);

    grid.attach(*managed_label("T. coeff", group), 0, top, 1, 1);
    grid.attach(_tile_temp_coefficient, 1, top++, 1, 1);
}

void TilesetEditor::setup_tile_props_actor(
    Grid &grid, Glib::RefPtr<Gtk::SizeGroup> group)
{
    int top = 0;

    grid.attach(*managed_label("Actor mode", group), 0, top, 1, 1);
    grid.attach(_tile_actor, 1, top++, 1, 1);
}

void TilesetEditor::setup_tile_toolbar(Toolbar &toolbar)
{
    toolbar.set_toolbar_style(TOOLBAR_ICONS);
    toolbar.set_icon_size(IconSize(ICON_SIZE_SMALL_TOOLBAR));
    {
        ToolButton *btn = manage(new ToolButton());
        RefPtr<Action> action = RefPtr<Action>::cast_dynamic(
            _root->get_builder()->get_object("action_tileset_new_tile"));
        btn->set_related_action(action);
        toolbar.append(*btn);
    }
    {
        ToolButton *btn = manage(new ToolButton());
        RefPtr<Action> action = RefPtr<Action>::cast_dynamic(
            _root->get_builder()->get_object("action_tileset_delete_tile"));
        btn->set_related_action(action);
        toolbar.append(*btn);
    }
}

TreeNodeChildren::iterator TilesetEditor::find_tile_row(const SharedTile &tile)
{
    TreeNodeChildren items = _tile_list->children();
    TreeNodeChildren::iterator it = items.begin();
    while (it != items.end()) {
        if (SharedTile((*it)[_tile_columns.col_tile]) == tile) {
            break;
        }
        it++;
    }
    return *it;
}

void TilesetEditor::flush_tile_props()
{
    std::unique_ptr<OperationGroup> ops(new OperationGroup());

    std::string display_name = _tile_display_name.get_text();
    if ((display_name != "") && (display_name != _current_tile->display_name)) {
        ops->add_operation(OperationPtr(new OpSetTileDisplayName(
            _editee, _current_tile, display_name)));
    }

    if (_tile_actor.get_active() != _current_tile->is_actor) {
        ops->add_operation(OperationPtr(new OpSetTileActor(
            _editee, _current_tile, _tile_actor.get_active())));
    }
    if (_tile_blocking.get_active() != _current_tile->is_blocking) {
        ops->add_operation(OperationPtr(new OpSetTileBlocking(
            _editee, _current_tile, _tile_blocking.get_active())));
    }
    if (_tile_destructible.get_active() != _current_tile->is_destructible) {
        ops->add_operation(OperationPtr(new OpSetTileDestructible(
            _editee, _current_tile, _tile_destructible.get_active())));
    }
    if (_tile_edible.get_active() != _current_tile->is_edible) {
        ops->add_operation(OperationPtr(new OpSetTileEdible(
            _editee, _current_tile, _tile_edible.get_active())));
    }
    if (_tile_gravity_affected.get_active() != _current_tile->is_gravity_affected) {
        ops->add_operation(OperationPtr(new OpSetTileGravityAffected(
            _editee, _current_tile, _tile_gravity_affected.get_active())));
    }
    if (_tile_movable.get_active() != _current_tile->is_movable) {
        ops->add_operation(OperationPtr(new OpSetTileMovable(
            _editee, _current_tile, _tile_movable.get_active())));
    }
    if (_tile_rollable.get_active() != _current_tile->is_rollable) {
        ops->add_operation(OperationPtr(new OpSetTileRollable(
            _editee, _current_tile, _tile_rollable.get_active())));
    }
    if (_tile_sticky.get_active() != _current_tile->is_sticky) {
        ops->add_operation(OperationPtr(new OpSetTileSticky(
            _editee, _current_tile, _tile_sticky.get_active())));
    }

    if (_tile_roll_radius.get_value() != _current_tile->roll_radius) {
        ops->add_operation(OperationPtr(new OpSetTileRollRadius(
            _editee, _current_tile, _tile_roll_radius.get_value())));
    }
    if (_tile_temp_coefficient.get_value() != _current_tile->temp_coefficient) {
        ops->add_operation(OperationPtr(new OpSetTileTempCoefficient(
            _editee, _current_tile, _tile_temp_coefficient.get_value())));
    }

    if (!ops->empty()) {
        _root->execute_operation(std::move(ops));
    }
}

SharedTile TilesetEditor::get_selected_tile()
{
    TreeModel::Path path;
    TreeViewColumn *dummy;
    _tile_list_view.get_cursor(path, dummy);

    ListStore::iterator iter = _tile_list->get_iter(path);
    if (iter == _tile_list->children().end()) {
        return nullptr;
    }

    TreeRow row = *iter;
    return SharedTile(row[_tile_columns.col_tile]);
}

void TilesetEditor::select_tile(const SharedTile &tile)
{
    if (_current_tile) {

    }
    _current_tile = tile;
    _tile_editor.switch_tile(_current_tile);
    if (_current_tile) {
        _tile_props.set_sensitive(true);

        update_tile_props();
    } else {
        _tile_props.set_sensitive(false);
    }
}

void TilesetEditor::update_tile_props()
{
    _updating = true;
    _conn_tile_changed.block();
    _tile_unique_name.set_text(_current_tile->unique_name);
    _tile_display_name.set_text(_current_tile->display_name);
    _tile_actor.set_active(_current_tile->is_actor);
    _tile_blocking.set_active(_current_tile->is_blocking);
    _tile_destructible.set_active(_current_tile->is_destructible);
    _tile_edible.set_active(_current_tile->is_edible);
    _tile_gravity_affected.set_active(_current_tile->is_gravity_affected);
    _tile_movable.set_active(_current_tile->is_movable);
    _tile_rollable.set_active(_current_tile->is_rollable);
    _tile_sticky.set_active(_current_tile->is_sticky);
    _tile_roll_radius.set_value(_current_tile->roll_radius);
    _tile_temp_coefficient.set_value(_current_tile->temp_coefficient);
    _conn_tile_changed.unblock();
    _updating = false;
}

void TilesetEditor::action_edit_details()
{
    _root->get_dlg_tileset_details()->edit_tileset(_editee->editee());
    _editee->signal_changed()(_editee);
}

void TilesetEditor::action_new_tile()
{
    NewTile *dlg = _root->get_dlg_new_tile();

    sigc::connection conn = dlg->signal_check_name().connect(
        sigc::mem_fun(*_editee, &TilesetEditee::check_unique_name));
    std::string unique_name = dlg->get_unique_name();
    conn.disconnect();

    if (unique_name == "") {
        return;
    }

    _root->execute_operation(
        OperationPtr(new OpNewTile(_editee, unique_name)));
}

void TilesetEditor::action_duplicate_tile()
{
    DuplicateTile *dlg = _root->get_dlg_duplicate_tile();

    SharedTile tile = get_selected_tile();
    if (!tile) {
        return;
    }

    std::string unique_name = tile->unique_name + "_clone";
    bool rewrite_references_to_self = true;

    sigc::connection conn = dlg->signal_check_name().connect(
        sigc::mem_fun(*_editee, &TilesetEditee::check_unique_name));
    dlg->get_duplicate_settings(
        unique_name,
        rewrite_references_to_self);
    conn.disconnect();

    _root->execute_operation(
        OperationPtr(new OpDuplicateTile(
            _editee,
            tile,
            unique_name,
            rewrite_references_to_self)));
}

void TilesetEditor::action_delete_tile()
{
    SharedTile tile = get_selected_tile();
    if (!tile) {
        return;
    }

    _root->execute_operation(
        OperationPtr(new OpDeleteTile(_editee, tile)));
}

void TilesetEditor::action_import_visual()
{
    if (!_current_tile) {
        return;
    }

    RefPtr<Gdk::Pixbuf> pixbuf = _root->get_dlg_open_image()->select_image();
    std::vector<FrameData> &frames = _current_tile->default_visual.frames;
    if (frames.empty()) {
        frames.push_back(FrameData());
    }
    if (!pixbuf_to_tile_image_data(pixbuf, &frames[0].image)) {
        message_dlg(*_root,
            "Could not open visual",
            "The visual has an incompatible format or is larger than "
            "65535 pixels on one axis.",
            MESSAGE_ERROR,
            BUTTONS_OK);

    } else {
        _editee->tile_changed(_current_tile);
    }
}

void TilesetEditor::editee_tile_changed(
        TilesetEditee *editee,
        const SharedTile &tile)
{
    TreeNodeChildren::iterator row_iter = find_tile_row(tile);
    assert(row_iter != _tile_list->children().end());
    TreeRow row = *row_iter;
    row[_tile_columns.col_display_name] = tile->display_name;
    if (tile == _current_tile) {
        update_tile_props();
        _tile_editor.update_cell_stamp();
    }
}

void TilesetEditor::editee_tile_created(
        TilesetEditee *editee,
        const SharedTile &tile)
{
    TreeRow row = *_tile_list->append();
    row[_tile_columns.col_tile] = tile;
    row[_tile_columns.col_display_name] = tile->display_name;
    select_tile(tile);
}

void TilesetEditor::editee_tile_deleted(
        TilesetEditee *editee,
        const SharedTile &tile)
{
    TreeNodeChildren::iterator row_iter = find_tile_row(tile);
    assert(row_iter != _tile_list->children().end());
    _tile_list->erase(row_iter);
    if (tile == _current_tile) {
        // force _current_tile to nullptr, to avoid updating a deleted
        // tile
        _current_tile = nullptr;
        select_tile(nullptr);
    }
}

void TilesetEditor::switch_button_button_press(
    GdkEventButton *event,
    Switch *button)
{
    if (event->button <= 3) {
        button->grab_focus();
    }
}

void TilesetEditor::tile_list_view_row_activated(
        const TreeModel::Path &path,
        TreeViewColumn *column)
{
    ListStore::iterator iter = _tile_list->get_iter(path);
    assert(iter != _tile_list->children().end());

    SharedTile tile = SharedTile((*iter)[_tile_columns.col_tile]);
    select_tile(tile);
}

const std::string &TilesetEditor::get_name() const
{
    return _editee->get_name();
}

std::string TilesetEditor::get_tab_name() const
{
    std::string name = get_name();
    if (name == "") {
        name = "[unnamed]";
    }
    return "🀰 "+name;
}

void TilesetEditor::set_name(const std::string &name)
{
    _editee->set_name(name);
}

void TilesetEditor::disable()
{
    _root->disable_tileset_controls();
    _conn_delete_tile.disconnect();
    _conn_duplicate_tile.disconnect();
    _conn_edit_details.disconnect();
    _conn_new_tile.disconnect();
    _conn_import_visual.disconnect();
}

void TilesetEditor::enable()
{
    _root->enable_tileset_controls();
    _conn_delete_tile =
        bind_action(
            _root->get_builder(), "action_tileset_delete_tile",
            sigc::mem_fun(*this, &TilesetEditor::action_delete_tile));

    _conn_duplicate_tile =
        bind_action(
            _root->get_builder(), "action_tileset_duplicate_tile",
            sigc::mem_fun(*this, &TilesetEditor::action_duplicate_tile));

    _conn_edit_details =
        bind_action(
            _root->get_builder(), "action_tileset_edit_details",
            sigc::mem_fun(*this, &TilesetEditor::action_edit_details));

    _conn_new_tile =
        bind_action(
            _root->get_builder(), "action_tileset_new_tile",
            sigc::mem_fun(*this, &TilesetEditor::action_new_tile));

    _conn_import_visual =
        bind_action(
            _root->get_builder(), "action_tileset_import_visual",
            sigc::mem_fun(*this, &TilesetEditor::action_import_visual));

}

void TilesetEditor::file_save(const PyEngine::StreamHandle &stream)
{
    if (_current_tile) {
        flush_tile_props();
    }
    save_tileset_to_stream(*_editee->editee(), stream);
}
