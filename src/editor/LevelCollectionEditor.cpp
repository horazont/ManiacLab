/**********************************************************************
File name: LevelCollectionEditor.cpp
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
#include "LevelCollectionEditor.hpp"

#include "GTKUtils.hpp"
#include "RootWindow.hpp"
#include "LevelCollectionOperations.hpp"

using namespace Glib;
using namespace Gtk;

/* LevelListModelColumns */

LevelCollectionEditor::LevelListModelColumns::LevelListModelColumns():
    TreeModelColumnRecord(),
    col_level(),
    col_uuid(),
    col_display_name()
{
    add(col_level);
    add(col_uuid);
    add(col_display_name);
}

/* TileListModelColumns */

LevelCollectionEditor::TileListModelColumns::TileListModelColumns():
    TreeModelColumnRecord(),
    col_tile(),
    col_display_name()
{
    add(col_tile);
    add(col_display_name);
}

/* utility functions */

inline void add_action_tool_button(
    RootWindow *root,
    Toolbar *toolbar,
    const std::string &action_name)
{
    ToolButton *btn = manage(new ToolButton());
    RefPtr<Action> action = RefPtr<Action>::cast_dynamic(
        root->get_builder()->get_object(action_name));
    btn->set_related_action(action);
    toolbar->append(*btn);
}

/* LevelCollectionEditor */

LevelCollectionEditor::LevelCollectionEditor(
        RootWindow *root,
        Container *parent,
        LevelCollectionEditee *editee):
    Editor(root, parent),
    _editee(editee),
    _conn_delete_level(),
    _conn_move_level_down(),
    _conn_move_level_up(),
    _conn_new_level(),
    _level_columns(),
    _level_list(ListStore::create(_level_columns)),
    _level_list_view(_level_list),
    _tile_columns(),
    _tile_list(ListStore::create(_tile_columns)),
    _tile_list_view(_tile_list)
{
    editee->signal_level_created().connect(
        sigc::mem_fun(*this, &LevelCollectionEditor::editee_level_created));
    editee->signal_level_deleted().connect(
        sigc::mem_fun(*this, &LevelCollectionEditor::editee_level_deleted));

    ScrolledWindow *level_editor_scroll = manage(new ScrolledWindow());

    VPaned *right_split = manage(new VPaned());
    {
        Box *vbox = manage(new Box(ORIENTATION_VERTICAL));

        _level_list_view.append_column(
            "Display name",
            _level_columns.col_display_name);

        ScrolledWindow *level_list_scroll = manage(new ScrolledWindow());
        level_list_scroll->add(_level_list_view);
        vbox->pack_start(*level_list_scroll, true, true);

        Toolbar *level_toolbar = manage(new Toolbar());
        level_toolbar->set_toolbar_style(TOOLBAR_ICONS);
        level_toolbar->set_icon_size(ICON_SIZE_SMALL_TOOLBAR);
        add_action_tool_button(
            root,
            level_toolbar,
            "action_level_collection_new_level");
        add_action_tool_button(
            root,
            level_toolbar,
            "action_level_collection_delete_level");
        add_action_tool_button(
            root,
            level_toolbar,
            "action_level_collection_move_level_up");
        add_action_tool_button(
            root,
            level_toolbar,
            "action_level_collection_move_level_down");

        vbox->pack_start(*level_toolbar, false, false);

        right_split->pack1(*vbox);
    }
    {
        ScrolledWindow *tile_list_scroll = manage(new ScrolledWindow());
        tile_list_scroll->add(_tile_list_view);
        right_split->pack2(*tile_list_scroll);
    }

    HPaned *main_split = manage(new HPaned());
    main_split->pack1(*level_editor_scroll, true, true);
    main_split->pack2(*right_split);
    main_split->set_hexpand(true);

    parent->add(*main_split);
    parent->show_all_children();


    /*
      +---------------------------------+------+
      |                                 |Level |
      |                                 |list  |
      |                                 |      |
      |   Visual level editor           |      |
      |                                 |      |
      |                                 |btns  |
      |                                 +------+
      |                                 |Tile  |
      |                                 |list  |
      |                                 |      |
      |                                 |      |
      |                                 |      |
      +---------------------------------+------+
     */

}

TreeNodeChildren::iterator LevelCollectionEditor::find_level_row(
    const SharedLevel &level)
{
    TreeNodeChildren items = _level_list->children();
    TreeNodeChildren::iterator it = items.begin();
    while (it != items.end()) {
        if (SharedLevel((*it)[_level_columns.col_level]) == level)
        {
            break;
        }
        ++it;
    }
    return *it;
}

SharedLevel LevelCollectionEditor::get_selected_level()
{
    TreeModel::Path path;
    TreeViewColumn *dummy;
    _level_list_view.get_cursor(path, dummy);

    if (path.size() == 0) {
        return nullptr;
    }

    ListStore::iterator iter = _level_list->get_iter(path);
    if (iter == _level_list->children().end()) {
        return nullptr;
    }

    TreeRow row = *iter;
    return SharedLevel(row[_level_columns.col_level]);
}

void LevelCollectionEditor::select_level(const SharedLevel &level)
{

}

const std::string &LevelCollectionEditor::get_name() const
{
    return _editee->get_name();
}

std::string LevelCollectionEditor::get_tab_name() const
{
    std::string name = get_name();
    if (name == "") {
        name = "[unnamed]";
    }
    return "▦ "+name;
}

std::string LevelCollectionEditor::get_vfs_dirname() const
{
    return "/data/levels";
}

void LevelCollectionEditor::set_name(const std::string &name)
{
    _editee->set_name(name);
}

void LevelCollectionEditor::action_delete_level()
{
    SharedLevel level = get_selected_level();
    if (!level) {
        return;
    }

    _root->execute_operation(
        OperationPtr(new OpDeleteLevel(_editee, level)));
}

void LevelCollectionEditor::action_move_level_down()
{

}

void LevelCollectionEditor::action_move_level_up()
{

}

void LevelCollectionEditor::action_new_level()
{
    _root->execute_operation(
        OperationPtr(new OpNewLevel(_editee)));
}

void LevelCollectionEditor::editee_level_created(
    LevelCollectionEditee *editee, const SharedLevel &level)
{
    TreeRow row = *_level_list->append();
    row[_level_columns.col_level] = level;
    row[_level_columns.col_uuid] = level->get_uuid().to_string();
    row[_level_columns.col_display_name] = level->get_display_name();
    select_level(level);
}

void LevelCollectionEditor::editee_level_deleted(
    LevelCollectionEditee *editee, const SharedLevel &level)
{
    TreeNodeChildren::iterator row_iter = find_level_row(level);
    assert(row_iter != _level_list->children().end());
    _level_list->erase(row_iter);
}

void LevelCollectionEditor::disable()
{
    _root->disable_level_controls();
    _conn_new_level.disconnect();
    _conn_delete_level.disconnect();
    _conn_move_level_down.disconnect();
    _conn_move_level_up.disconnect();
}

void LevelCollectionEditor::enable()
{
    _root->enable_level_controls();
    _conn_new_level = bind_action(
        _root->get_builder(), "action_level_collection_new_level",
        sigc::mem_fun(*this, &LevelCollectionEditor::action_new_level));
    _conn_delete_level = bind_action(
        _root->get_builder(), "action_level_collection_delete_level",
        sigc::mem_fun(*this, &LevelCollectionEditor::action_delete_level));
    _conn_move_level_down = bind_action(
        _root->get_builder(), "action_level_collection_move_level_down",
        sigc::mem_fun(*this, &LevelCollectionEditor::action_move_level_down));
    _conn_move_level_up = bind_action(
        _root->get_builder(), "action_level_collection_move_level_up",
        sigc::mem_fun(*this, &LevelCollectionEditor::action_move_level_up));
}

void LevelCollectionEditor::file_save(const PyEngine::StreamHandle &stream)
{
    _editee->editee()->save_to_stream(
        stream,
        std::bind(
            &RootWindow::get_tileset_name,
            _root,
            std::placeholders::_1));
}
