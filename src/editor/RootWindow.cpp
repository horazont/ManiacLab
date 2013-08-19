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

#include "io/Data.hpp"

#include "GTKUtils.hpp"

using namespace Gtk;
using namespace Glib;
using namespace StructStream;
using namespace PyEngine;

inline void bind_action(
    const RefPtr<Builder> &builder,
    const std::string &name,
    const Action::SlotActivate &slot,
    RefPtr<Action> *action_dest = nullptr)
{
    RefPtr<Action> action;
    action = action.cast_dynamic(builder->get_object(name));
    action->signal_activate().connect(slot);
    if (action_dest) {
        *action_dest = action;
    }
}

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
    _editors(),
    _current_editor(nullptr),
    _builder(builder),
    _menu_level(nullptr),
    _menu_tileset(nullptr),
    _actions_level(_actions_level.cast_dynamic(
        builder->get_object("actions_level"))),
    _actions_tileset(_actions_tileset.cast_dynamic(
        builder->get_object("actions_tileset"))),
    _tabs(nullptr),
    _action_save(),
    _action_save_as(),
    _dlg_create_tileset(nullptr),
    _dlg_open_file(nullptr),
    _dlg_save_file(nullptr),
    _dlg_tileset_details(nullptr)
{
    _builder->get_widget("menu_tileset", _menu_tileset);
    _menu_tileset->hide();

    _builder->get_widget("menu_level", _menu_level);
    _menu_level->hide();

    RefPtr<Action> action;

    /* Configure actions */

    bind_action(
        _builder, "action_file_new_tileset",
        sigc::mem_fun(*this, &RootWindow::action_file_new_tileset));

    bind_action(
        _builder, "action_file_open",
        sigc::mem_fun(*this, &RootWindow::action_file_open));

    bind_action(
        _builder, "action_file_save",
        sigc::mem_fun(*this, &RootWindow::action_file_save),
        &_action_save);

    bind_action(
        _builder, "action_file_save_as",
        sigc::mem_fun(*this, &RootWindow::action_file_save_as),
        &_action_save_as);

    bind_action(
        _builder, "action_file_quit",
        sigc::mem_fun(*this, &RootWindow::action_file_quit));


    bind_action(
        _builder, "action_tileset_edit_details",
        sigc::mem_fun(*this, &RootWindow::action_tileset_edit_details));

    /* End of configure actions */

    _builder->get_widget("notebook_editors", _tabs);
    _tabs->signal_switch_page().connect(
        sigc::mem_fun(*this, &RootWindow::tabs_switch_page));

    _builder->get_widget("dlg_open_file", _dlg_open_file);
    _dlg_open_file->signal_file_activated().connect(
        sigc::mem_fun(*this, &RootWindow::dlg_open_file_activate));
    _dlg_open_file->signal_response().connect(
        sigc::mem_fun(*this, &RootWindow::dlg_open_file_response));
    _dlg_open_file->set_action(FILE_CHOOSER_ACTION_OPEN);

    _builder->get_widget("dlg_save_file", _dlg_save_file);
    _dlg_save_file->signal_file_activated().connect(
        sigc::mem_fun(*this, &RootWindow::dlg_save_file_activate));
    _dlg_save_file->signal_response().connect(
        sigc::mem_fun(*this, &RootWindow::dlg_save_file_response));
    _dlg_save_file->set_action(FILE_CHOOSER_ACTION_SAVE);
    _dlg_save_file->set_do_overwrite_confirmation();

    _builder->get_widget_derived("dlg_tileset_details", _dlg_tileset_details);

    {
        RefPtr<FileFilter> filter = FileFilter::create();
        filter->set_name("ManiacLab files (*.tileset; *.level)");
        filter->add_pattern("*.tileset");
        filter->add_pattern("*.level");
        _dlg_open_file->add_filter(filter);
    }

    {
        RefPtr<FileFilter> filter = FileFilter::create();
        filter->set_name("All files");
        filter->add_pattern("*");
        _dlg_open_file->add_filter(filter);
    }

    _builder->get_widget("dlg_create_tileset", _dlg_create_tileset);
    _dlg_create_tileset->signal_response().connect(
        sigc::mem_fun(*this, &RootWindow::dlg_create_tileset_response));

    Entry *entry;
    _builder->get_widget("create_tileset_unique_name", entry);
    entry->signal_activate().connect(
        sigc::mem_fun(*this, &RootWindow::dlg_create_tileset_activate));

    switch_editor(nullptr);
}

RootWindow::~RootWindow()
{
    for (auto item: _editors) {
        delete item;
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
    Entry *entry;
    _builder->get_widget("create_tileset_unique_name", entry);
    entry->set_text("");
    _dlg_create_tileset->show();
}

void RootWindow::action_file_open()
{
    _dlg_open_file->unselect_all();
    _dlg_open_file->run();
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
    _dlg_save_file->unselect_all();
    _dlg_save_file->run();
}

void RootWindow::action_file_quit()
{
    hide();
}

void RootWindow::action_tileset_edit_details()
{
    TilesetEditor *editor =
        dynamic_cast<TilesetEditor*>(_current_editor);
    assert(editor);
    SharedTileset tileset = editor->editee()->editee();

    _dlg_tileset_details->edit_tileset(tileset);
}

void RootWindow::dlg_create_tileset_activate()
{
    dlg_create_tileset_response(2);
}

void RootWindow::dlg_create_tileset_response(int response_id)
{
    switch (response_id) {
    case 2:
    {
        std::string unique_name;
        {
            Entry *entry;
            _builder->get_widget("create_tileset_unique_name", entry);
            unique_name = entry->get_text();
        }

        if (unique_name == "") {
            message_dlg(*this,
                "Invalid unique name",
                "The unique name must not be empty.",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }

        if (_tileset_by_name.find(unique_name) != _tileset_by_name.end()) {
            message_dlg(*this,
                "Conflict",
                "The unique name “" + unique_name + "” is already in use.",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }

        _dlg_create_tileset->hide();

        SharedTileset new_tileset(new TilesetData());
        new_tileset->header.unique_name = unique_name;
        new_tileset->header.display_name = unique_name;
        get_tileset_editor(make_tileset_editable(new_tileset));
        break;
    }
    case 1:
    default:
        _dlg_create_tileset->hide();
        break;
    };
}

void RootWindow::dlg_open_file_activate()
{
    _dlg_open_file->hide();
    open_file(_dlg_open_file->get_filename());
}

void RootWindow::dlg_open_file_response(int response_id)
{
    switch (response_id) {
    case 2:
    {
        const std::string &filename = _dlg_open_file->get_filename();
        if (filename == "") {
            message_dlg(*this,
                "No file selected",
                "You have to select a file to open a file.",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }
        _dlg_open_file->hide();
        open_file(filename);
        break;
    }
    case 1:
    default:
        _dlg_open_file->hide();
        break;
    };
}

void RootWindow::dlg_save_file_activate()
{
    _dlg_save_file->hide();
    save_file(_dlg_save_file->get_filename());
}

void RootWindow::dlg_save_file_response(int response_id)
{
    switch (response_id) {
    case 2:
    {
        const std::string &filename = _dlg_save_file->get_filename();
        if (filename == "") {
            message_dlg(*this,
                "No file selected",
                "You have to select a file to open a file.",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }
        _dlg_save_file->hide();
        save_file(filename);
        break;
    }
    case 1:
    default:
        _dlg_open_file->hide();
        break;
    };
}

void RootWindow::tabs_switch_page(Widget *page, guint page_num)
{
    Editor *new_editor = _editors.at(page_num);
    if (new_editor == _current_editor) {
        return;
    }

    switch_editor(new_editor);
}

void RootWindow::open_file(const std::string &filename)
{
    ContainerHandle root;
    FileType filet;

    std::tie(root, filet) = load_tree_from_stream(
        StreamHandle(new FileStream(filename, OM_READ, WM_IGNORE, SM_ALLOW_READ)));

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
        assert(root);
        std::unique_ptr<TilesetData> tileset = load_tileset_from_tree(root);
        assert(tileset);
        editor = get_tileset_editor(make_tileset_editable(
            SharedTileset(tileset.release())));
        break;
    }
    case FT_LEVEL:
    {
        assert(root);
        error_msg = "Cannot load levels (yet :)).";
        error = true;
        break;
    }
    default:
    {
        error_msg = "Unexpected file type value.";
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

void RootWindow::save_file(const std::string &filename)
{
    assert(_current_editor);
    StreamHandle stream(new FileStream(filename, OM_WRITE, WM_OVERWRITE, SM_EXCLUSIVE));
    _current_editor->file_save(stream);
}

void RootWindow::switch_editor(Editor *new_editor)
{
    if (_current_editor) {
        _current_editor->disable();
    }
    _current_editor = new_editor;
    if (_current_editor) {
        _current_editor->enable();

        _action_save->set_sensitive(true);
        _action_save_as->set_sensitive(true);
    } else {
        _action_save->set_sensitive(false);
        _action_save_as->set_sensitive(false);
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

TilesetEditor *RootWindow::get_tileset_editor(TilesetEditee *editee)
{
    auto editee_it = _loaded_tilesets.find(editee->editee().get());
    assert(editee_it != _loaded_tilesets.end());

    auto it = _tileset_editors.find(editee->editee().get());
    if (it != _tileset_editors.end()) {
        return (*it).second;
    }

    Gtk::Box *editor_box = manage(new Box());
    TilesetEditor *editor = new TilesetEditor(this, editor_box, editee);
    _editors.push_back(editor);
    _tabs->append_page(*editor_box, editee->editee()->header.unique_name + " (Tileset)");
    editor_box->show();
    _tileset_editors[editee->editee().get()] = editor;

    return editor;
}

TilesetEditee *RootWindow::make_tileset_editable(const SharedTileset &data)
{
    {
        auto it = _loaded_tilesets.find(data.get());
        if (it != _loaded_tilesets.end()) {
            return (*it).second;
        }
    }

    {
        auto it = _tileset_by_name.find(data->header.unique_name);
        if (it != _tileset_by_name.end()) {
            throw std::invalid_argument("Duplicate tileset unique_name: "+data->header.unique_name);
        }
    }

    TilesetEditee *editee = new TilesetEditee(data);
    _loaded_tilesets[data.get()] = editee;
    _tileset_by_name[data->header.unique_name] = data;
    return editee;
}
