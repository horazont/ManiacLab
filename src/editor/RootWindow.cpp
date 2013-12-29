/**********************************************************************
File name: RootWindow.cpp
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
#include "RootWindow.hpp"

#include <CEngine/IO/FileStream.hpp>
#include <CEngine/IO/Log.hpp>
#include <CEngine/VFS/Utils.hpp>
#include <CEngine/VFS/Errors.hpp>

#include "io/Data.hpp"

#include "GTKUtils.hpp"

using namespace Gtk;
using namespace Glib;
using namespace StructStream;
using namespace PyEngine;

inline std::string get_entry_text(
    const RefPtr<Builder> &builder,
    const std::string &name)
{
    Entry *entry;
    builder->get_widget(name, entry);
    return entry->get_text();
}

inline std::string get_textview_text(
    const RefPtr<Builder> &builder,
    const std::string &name)
{
    TextView *entry;
    builder->get_widget(name, entry);
    return entry->get_buffer()->get_text();
}

/* RootWindow */

RootWindow::RootWindow(
        BaseObjectType *cobject,
        const Glib::RefPtr<Gtk::Builder> &builder):
    Window(cobject),
    _tileset_by_name(),
    _loaded_tilesets(),
    _tileset_editors(),
    _level_collection_by_name(),
    _loaded_level_collections(),
    _level_collection_editors(),
    _editors(),
    _current_editor(nullptr),
    _undo_stack(),
    _redo_stack(),
    _vfs(),
    _builder(builder),
    _menu_level(nullptr),
    _menu_tileset(nullptr),
    _actions_level(_actions_level.cast_dynamic(
        builder->get_object("actions_level"))),
    _actions_level_collection(_actions_level_collection.cast_dynamic(
        builder->get_object("actions_level_collection"))),
    _actions_tileset(_actions_tileset.cast_dynamic(
        builder->get_object("actions_tileset"))),
    _accel_level(_accel_level.cast_dynamic(
        builder->get_object("accel_level"))),
    _accel_tileset(_accel_tileset.cast_dynamic(
        builder->get_object("accel_tileset"))),
    _action_undo(_action_undo.cast_dynamic(
        builder->get_object("action_edit_undo"))),
    _action_redo(_action_redo.cast_dynamic(
        builder->get_object("action_edit_redo"))),
    _tabs(nullptr),
    _action_file_save(),
    _action_file_save_as(),
    _action_file_close(),
    _dlg_open_image(nullptr),
    _dlg_tileset_details(nullptr),
    _dlg_open_vfs_file(&_vfs, false),
    _dlg_save_vfs_file(&_vfs, true)
{
    _vfs.mount(
        "/data",
        std::move(MountPtr(new MountDirectory("data", false))),
        MountPriority::FileSystem);


    _builder->get_widget("menu_tileset", _menu_tileset);
    _menu_tileset->hide();

    _builder->get_widget("menu_level", _menu_level);
    _menu_level->hide();

    _builder->get_widget("menu_level_collection", _menu_level_collection);
    _menu_level_collection->hide();

    add_accel_group(RefPtr<AccelGroup>::cast_dynamic(
        builder->get_object("accel_common")));

    {
        RefPtr<Action> action = RefPtr<Action>::cast_dynamic(
            _builder->get_object("action_file_quit"));
        action->disconnect_accelerator();
    }

    /* Configure actions */

    bind_action(
        _builder, "action_file_new_tileset",
        sigc::mem_fun(*this, &RootWindow::action_file_new_tileset));

    bind_action(
        _builder, "action_file_new_level",
        sigc::mem_fun(*this, &RootWindow::action_file_new_level));

    bind_action(
        _builder, "action_file_open",
        sigc::mem_fun(*this, &RootWindow::action_file_open));

    bind_action(
        _builder, "action_file_save",
        sigc::mem_fun(*this, &RootWindow::action_file_save),
        &_action_file_save);

    bind_action(
        _builder, "action_file_save_as",
        sigc::mem_fun(*this, &RootWindow::action_file_save_as),
        &_action_file_save_as);

    bind_action(
        _builder, "action_file_close",
        sigc::mem_fun(*this, &RootWindow::action_file_close),
        &_action_file_close);

    bind_action(
        _builder, "action_file_quit",
        sigc::mem_fun(*this, &RootWindow::action_file_quit));


    bind_action(
        _builder, "action_help_about",
        sigc::mem_fun(*this, &RootWindow::action_help_about));

    _action_undo->signal_activate().connect(
        sigc::mem_fun(*this, &RootWindow::action_edit_undo));

    _action_redo->signal_activate().connect(
        sigc::mem_fun(*this, &RootWindow::action_edit_redo));


    /* End of configure actions */

    _builder->get_widget("notebook_editors", _tabs);
    _tabs->signal_switch_page().connect(
        sigc::mem_fun(*this, &RootWindow::tabs_switch_page));

    _builder->get_widget_derived("dlg_open_image", _dlg_open_image);
    _dlg_open_image->set_action(FILE_CHOOSER_ACTION_OPEN);

    _builder->get_widget_derived("dlg_new_tile", _dlg_new_tile);
    _builder->get_widget_derived("dlg_duplicate_tile", _dlg_duplicate_tile);
    _builder->get_widget_derived("dlg_tileset_details", _dlg_tileset_details);

    _dlg_open_vfs_file.set_title("Open VFS file…");
    _dlg_save_vfs_file.set_title("Save VFS file…");

    _builder->get_widget("dlg_about", _dlg_about);

    switch_editor(nullptr);
    update_undo_redo_sensitivity();
}

RootWindow::~RootWindow()
{
    while (_editors.size() > 0) {
        close_editor(_editors[_editors.size()-1]);
    }
    _editors.clear();
    _tileset_editors.clear();
    for (auto item: _loaded_tilesets) {
        delete item.second;
    }
    _loaded_tilesets.clear();
}

void RootWindow::action_file_new_tileset()
{
    SharedTileset tileset(new TilesetData());
    tileset->header.display_name = "unnamed";
    get_tileset_editor(make_tileset_editable(tileset, ""));
}

void RootWindow::action_file_new_level()
{
    SharedLevelCollection collection(new LevelCollection());
    collection->display_name = "unnamed";
    get_level_collection_editor(
        make_level_collection_editable(collection, ""));
}

void RootWindow::action_file_open()
{
    std::string path = _dlg_open_vfs_file.select_file_multisource({
        "/data/tilesets",
        "/data/levels"
        });
    if (path == "") {
        return;
    }
    open_file(path);
}

void RootWindow::action_file_save()
{
    assert(_current_editor);
    if (!_current_editor->get_filename_known()) {
        action_file_save_as();
        return;
    }

    save_file(_current_editor->get_filename());
}

void RootWindow::action_file_save_as()
{
    std::string path = _dlg_save_vfs_file.select_file(
        _current_editor->get_vfs_dirname());

    if (path == "") {
        return;
    }
    save_file(path);
}

void RootWindow::action_file_close()
{
    assert(_current_editor);
    close_editor(_current_editor);
}

void RootWindow::action_file_quit()
{
    hide();
}

void RootWindow::action_edit_undo()
{
    if (_undo_stack.empty()) {
        return;
    }

    OperationPtr &operation = _undo_stack.back();
    operation->undo();
    _redo_stack.push_back(std::move(operation));
    _undo_stack.pop_back();

    update_undo_redo_sensitivity();
}

void RootWindow::action_edit_redo()
{
    if (_redo_stack.empty()) {
        return;
    }

    OperationPtr &operation = _redo_stack.back();
    operation->execute();
    _undo_stack.push_back(std::move(operation));
    _redo_stack.pop_back();

    update_undo_redo_sensitivity();
}

void RootWindow::action_help_about()
{
    _dlg_about->run();
}

void RootWindow::tabs_switch_page(Widget *page, guint page_num)
{
    Editor *new_editor = _editors.at(page_num);
    if (new_editor == _current_editor) {
        return;
    }

    switch_editor(new_editor);
}

void RootWindow::delete_editor(Editor *editor)
{
    // we must switch away to avoid segfault; we don't know the new
    // editor yet (if one exists at all), so we switch to nullptr.
    switch_editor(nullptr);

    {
        auto it = std::find(_editors.begin(), _editors.end(), editor);
        assert(it != _editors.end());
        _editors.erase(it);
    }

    _tabs->remove_page(*editor->get_parent());
    delete editor;
}

Gtk::Box *RootWindow::new_editor_box()
{
    Gtk::Box *editor_box = manage(new Box());
    editor_box->set_hexpand(true);
    editor_box->set_vexpand(true);
    return editor_box;
}

void RootWindow::update_undo_redo_sensitivity()
{
    _action_undo->set_sensitive(!_undo_stack.empty());
    _action_redo->set_sensitive(!_redo_stack.empty());
}

void RootWindow::open_file(const std::string &filename)
{
    ContainerHandle header_root;
    FileType filet;
    StreamHandle stream = _vfs.open(filename, OM_READ);

    std::tie(header_root, filet) = load_header_from_stream(stream);

    std::string error_msg;
    bool error = false;

    Editor *editor = nullptr;

    switch (filet) {
    case FT_NO_STRUCTSTREAM:
    {
        error_msg = "The given file is not a StructStream (i.e. not a compatible file format).";
        error = true;
        break;
    }
    case FT_EMPTY:
    {
        error_msg = "The given file is empty.";
        error = true;
        break;
    }
    case FT_NO_MANIACLAB:
    {
        error_msg = "The file is not a ManiacLab file (or contains content of a newer version).";
        error = true;
        break;
    }
    case FT_TILESET:
    {
        const std::string name = basename(filename);
        {
            auto it = _tileset_by_name.find(name);
            if (it != _tileset_by_name.end())
            {
                SharedTileset tileset = (*it).second;
                auto editor_it = _tileset_editors.find(tileset.get());
                if (editor_it != _tileset_editors.end())
                {
                    error_msg = "Tileset already open.";
                    error = true;
                    break;
                }
                get_tileset_editor(_loaded_tilesets[tileset.get()]);
                break;
            }
        }
        assert(header_root);
        std::unique_ptr<TilesetData> tileset = complete_tileset_from_stream(header_root, stream);
        assert(tileset);
        try {
            editor = get_tileset_editor(
                make_tileset_editable(
                    SharedTileset(tileset.release()),
                    name));
        } catch (const std::exception &err) {
            error_msg = err.what();
            error = true;
        }
        break;
    }
    case FT_LEVEL_COLLECTION:
    {
        const std::string name = basename(filename);
        {
            auto it = _level_collection_by_name.find(name);
            if (it != _level_collection_by_name.end())
            {
                SharedLevelCollection collection = (*it).second;
                auto editor_it = _level_collection_editors.find(collection.get());
                if (editor_it != _level_collection_editors.end())
                {
                    error_msg = "Level collection already open.";
                    error = true;
                    break;
                }
                get_level_collection_editor(
                    _loaded_level_collections[collection.get()]);
                break;
            }
        }
        assert(header_root);
        std::unique_ptr<LevelCollection> collection;
        IOQuality quality;
        std::tie(collection, quality) =
            complete_level_collection_from_stream(
                header_root,
                stream,
                std::bind(
                    &RootWindow::lookup_tile,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2));
        assert(collection);
        try {
            editor = get_level_collection_editor(
                make_level_collection_editable(
                    SharedLevelCollection(collection.release()),
                    basename(filename)));
        } catch (const std::exception &err) {
            error_msg = err.what();
            error = true;
        }
        break;
    }
    default:
    {
        error_msg = "Unexpected file type value (this is an internal error; please report it to the developer!).";
        error = true;
        break;
    }
    };

    if (error) {
        message_dlg(*this,
            "Could not open file: "+filename,
            error_msg,
            MESSAGE_ERROR,
            BUTTONS_OK);
    } else {
        editor->set_filename(filename);
    }
}

void RootWindow::process_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        open_file(argv[i]);
    }
}

void RootWindow::save_file(const std::string &filename)
{
    assert(_current_editor);
    StreamHandle stream = _vfs.open(filename, OM_WRITE, WM_OVERWRITE);
    _current_editor->file_save(stream);
    _current_editor->set_filename(filename);
    std::string new_name = basename(filename);
    if (_current_editor->get_name() != new_name) {
        rename_editor(_current_editor, new_name);
    }
}

void RootWindow::switch_editor(Editor *new_editor)
{
    if (_current_editor) {
        _current_editor->disable();
    }
    _current_editor = new_editor;
    if (_current_editor) {
        _current_editor->enable();

        _action_file_save->set_sensitive(true);
        _action_file_save_as->set_sensitive(true);
        _action_file_close->set_sensitive(true);
    } else {
        _action_file_save->set_sensitive(false);
        _action_file_save_as->set_sensitive(false);
        _action_file_close->set_sensitive(false);
    }
}

const Glib::RefPtr<Gtk::Builder> &RootWindow::get_builder() const
{
    return _builder;
}

Gtk::MenuItem *RootWindow::get_menu_level()
{
    return _menu_level;
}

Gtk::MenuItem *RootWindow::get_menu_tileset()
{
    return _menu_tileset;
}

void RootWindow::disable_level_controls()
{
    remove_accel_group(_accel_level);
    _actions_level->set_sensitive(false);
    _actions_level_collection->set_sensitive(false);
    _menu_level->set_visible(false);
    _menu_level_collection->set_sensitive(false);
}

void RootWindow::disable_tileset_controls()
{
    remove_accel_group(_accel_tileset);
    _actions_tileset->set_sensitive(false);
    _menu_tileset->set_visible(false);
}

void RootWindow::enable_level_controls()
{
    _menu_level->set_visible(true);
    _menu_level_collection->set_visible(true);
    _actions_level->set_sensitive(true);
    _actions_level_collection->set_sensitive(true);
    add_accel_group(_accel_level);
}

void RootWindow::enable_tileset_controls()
{
    _menu_tileset->set_visible(true);
    _actions_tileset->set_sensitive(true);
    add_accel_group(_accel_tileset);
}

void RootWindow::execute_operation(OperationPtr &&operation)
{
    operation->execute();
    if (!operation->is_undoable()) {
        _undo_stack.clear();
    } else {
        _undo_stack.push_back(std::move(operation));
    }
    _redo_stack.clear();
    update_undo_redo_sensitivity();
}

void RootWindow::close_editor(Editor *editor)
{
    assert(editor);
    {
        TilesetEditor *tileset_editor =
            dynamic_cast<TilesetEditor*>(editor);
        if (tileset_editor) {
            close_tileset(tileset_editor->editee());
            return;
        }
    }
    {
        LevelCollectionEditor *collection_editor =
            dynamic_cast<LevelCollectionEditor*>(editor);
        if (collection_editor) {
            close_level_collection(collection_editor->editee());
            return;
        }
    }

    throw std::invalid_argument("Cannot handle editor object. "
                                "Unknown type.");

}

void RootWindow::close_level_collection(LevelCollectionEditee *editee)
{
    SharedLevelCollection data = editee->editee();
    auto it = _level_collection_editors.find(data.get());
    assert(it != _level_collection_editors.end());
    LevelCollectionEditor *editor = (*it).second;
    _level_collection_editors.erase(it);

    if (editee->get_name() != "") {
        auto it = _level_collection_by_name.find(editee->get_name());
        assert(it != _level_collection_by_name.end());
        _level_collection_by_name.erase(it);
    }

    delete_editor(editor);
}

void RootWindow::close_tileset(TilesetEditee *editee)
{
    SharedTileset data = editee->editee();
    auto it = _tileset_editors.find(data.get());
    assert(it != _tileset_editors.end());
    TilesetEditor *editor = (*it).second;
    _tileset_editors.erase(it);

    if (editee->get_name() != "") {
        auto it = _tileset_by_name.find(editee->get_name());
        assert(it != _tileset_by_name.end());
        _tileset_by_name.erase(it);
    }

    delete_editor(editor);
}

LevelCollectionEditor *RootWindow::get_level_collection_editor(
    LevelCollectionEditee *editee)
{
    {
        auto editee_it = _loaded_level_collections.find(editee->editee().get());
        assert(editee_it != _loaded_level_collections.end());
    }

    {
        auto it = _level_collection_editors.find(editee->editee().get());
        if (it != _level_collection_editors.end()) {
            return (*it).second;
        }
    }

    Gtk::Box *editor_box = new_editor_box();
    LevelCollectionEditor *editor = new LevelCollectionEditor(
        this, editor_box, editee);
    _editors.push_back(editor);
    _tabs->append_page(*editor_box, editor->get_tab_name());
    editor_box->show();
    _level_collection_editors[editee->editee().get()] = editor;

    return editor;
}

TilesetEditor *RootWindow::get_tileset_editor(TilesetEditee *editee)
{
    auto editee_it = _loaded_tilesets.find(editee->editee().get());
    assert(editee_it != _loaded_tilesets.end());

    auto it = _tileset_editors.find(editee->editee().get());
    if (it != _tileset_editors.end()) {
        return (*it).second;
    }

    Gtk::Box *editor_box = new_editor_box();
    TilesetEditor *editor = new TilesetEditor(this, editor_box, editee);
    _editors.push_back(editor);
    _tabs->append_page(*editor_box, editor->get_tab_name());
    editor_box->show();
    _tileset_editors[editee->editee().get()] = editor;

    return editor;
}

std::string RootWindow::get_tileset_name(const SharedTileset &tileset)
{
    TilesetEditee *editee = _loaded_tilesets[tileset.get()];
    if (!editee) {
        throw std::invalid_argument(
            "SharedTileset instance is not loaded anymore.");
    }

    if (editee->get_name() == "") {
        throw std::runtime_error("Tileset is unnamed");
    }

    return editee->get_name();
}

TilesetEditee *RootWindow::load_tileset_by_name(
    const std::string &name)
{
    {
        auto it = _tileset_by_name.find(name);
        if (it != _tileset_by_name.end()) {
            return _loaded_tilesets[(*it).second.get()];
        }
    }

    const std::string fullpath = "/data/tilesets/"+name;
    if (!_vfs.file_readable(fullpath)) {
        PyEngine::log->log(Information)
            << "load_tileset_by_name: vfs claims we cannot read " << fullpath
            << submit;
        return nullptr;
    }

    ContainerHandle header;
    FileType type;
    StreamHandle stream;
    try {
        stream = _vfs.open(fullpath, OM_READ);
    } catch (const VFSIOError &err) {
        PyEngine::log->log(Warning)
            << "load_tileset_by_name: opening of `" << fullpath << "' failed: "
            << err.what() << submit;
        return nullptr;
    }

    std::tie(header, type) =
        load_header_from_stream(stream);

    if (type != FT_TILESET) {
        PyEngine::log->log(Information)
            << "load_tileset_by_name: `" << fullpath << "' is not a tileset."
            << submit;
        return nullptr;
    }

    assert(header);
    std::unique_ptr<TilesetData> tileset =
        complete_tileset_from_stream(header, stream);
    assert(tileset);
    return make_tileset_editable(SharedTileset(tileset.release()), name);
}

LevelData::TileBinding RootWindow::lookup_tile(
    const std::string &tileset_name,
    const PyEngine::UUID &uuid)
{
    TilesetEditee *editee = load_tileset_by_name(tileset_name);
    if (!editee) {
        return std::make_pair(nullptr, nullptr);
    }

    SharedTile tile = editee->find_tile(uuid);
    return std::make_pair(editee->editee(), tile);
}

LevelCollectionEditee *RootWindow::make_level_collection_editable(
    const SharedLevelCollection &data, const std::string &name)
{
    {
        auto it = _loaded_level_collections.find(data.get());
        if (it != _loaded_level_collections.end()) {
            return (*it).second;
        }
    }

    {
        if (name != "") {
            auto it = _level_collection_by_name.find(name);
            if (it != _level_collection_by_name.end()) {
                throw std::invalid_argument(
                    "Duplicate level collection editee name: "+name);
            }
        }
    }

    LevelCollectionEditee *editee = new LevelCollectionEditee(data, name);
    _loaded_level_collections[data.get()] = editee;
    _level_collection_by_name[name] = data;
    return editee;
}

TilesetEditee *RootWindow::make_tileset_editable(
    const SharedTileset &data, const std::string &name)
{
    {
        auto it = _loaded_tilesets.find(data.get());
        if (it != _loaded_tilesets.end()) {
            return (*it).second;
        }
    }

    {
        if (name != "") {
            auto it = _tileset_by_name.find(name);
            if (it != _tileset_by_name.end()) {
                throw std::invalid_argument("Duplicate tileset editee name: "+name);
            }
        }
    }

    TilesetEditee *editee = new TilesetEditee(data, name);
    _loaded_tilesets[data.get()] = editee;
    _tileset_by_name[name] = data;
    return editee;
}

void RootWindow::rename_editor(
    Editor *editor, const std::string &new_name)
{
    assert(editor);
    {
        TilesetEditor *tileset_editor =
            dynamic_cast<TilesetEditor*>(editor);
        if (tileset_editor) {
            rename_tileset(tileset_editor->editee(), new_name);
            return;
        }
    }
    {
        LevelCollectionEditor *collection_editor =
            dynamic_cast<LevelCollectionEditor*>(editor);
        if (collection_editor) {
            rename_level_collection(collection_editor->editee(), new_name);
            return;
        }
    }

    throw std::invalid_argument("Cannot handle editor object. "
                                "Unknown type.");
}

void RootWindow::rename_level_collection(
    LevelCollectionEditee *editee, const std::string &new_name)
{
    bool is_null_name = (new_name == "");

    if (!is_null_name) {
        // we first check if a tileset with that name exists
        {
            auto it = _level_collection_by_name.find(new_name);
            if (it != _level_collection_by_name.end()) {
                // it exists, we have to rename it
                SharedLevelCollection other_collection = (*it).second;
                rename_level_collection(
                    _loaded_level_collections[other_collection.get()], "");
            }
        }
    }

    if (editee->get_name() != "") {
        auto it = _level_collection_by_name.find(editee->get_name());
        assert(it != _level_collection_by_name.end());
        _level_collection_by_name.erase(it);
    }

    editee->set_name(new_name);

    if (!is_null_name) {
        _level_collection_by_name[new_name] = editee->editee();
    }

    LevelCollectionEditor *editor =
        _level_collection_editors[editee->editee().get()];
    update_tab_name(editor);
}

void RootWindow::rename_tileset(
    TilesetEditee *editee, const std::string &new_name)
{
    bool is_null_name = (new_name == "");

    if (!is_null_name) {
        // we first check if a tileset with that name exists
        {
            auto it = _tileset_by_name.find(new_name);
            if (it != _tileset_by_name.end()) {
                // it exists, we have to rename it
                SharedTileset other_tileset = (*it).second;
                rename_tileset(
                    _loaded_tilesets[other_tileset.get()], "");
            }
        }
    }

    if (editee->get_name() != "") {
        auto it = _tileset_by_name.find(editee->get_name());
        assert(it != _tileset_by_name.end());
        _tileset_by_name.erase(it);
    }

    editee->set_name(new_name);

    if (!is_null_name) {
        _tileset_by_name[new_name] = editee->editee();
    }

    TilesetEditor *editor = _tileset_editors[editee->editee().get()];
    update_tab_name(editor);
}

void RootWindow::update_tab_name(Editor *editor)
{
    _tabs->set_tab_label_text(
        *editor->get_parent(),
        editor->get_tab_name());
}
