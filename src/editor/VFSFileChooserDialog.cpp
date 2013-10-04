#include "VFSFileChooserDialog.hpp"

#include <chrono>

#include <CEngine/IO/Log.hpp>
#include <CEngine/VFS/Utils.hpp>
#include <CEngine/VFS/Errors.hpp>

#include "GTKUtils.hpp"

#include "io/Data.hpp"

using namespace Glib;
using namespace Gtk;
using namespace PyEngine;

std::string format_file_size(uint64_t size)
{
    static const char *magnitude_prefixes[5] = {
        "",
        "ki",
        "Mi",
        "Gi",
        "Ti"
    };
    static const int array_length =
        sizeof(magnitude_prefixes) / sizeof(const char*);

    int magnitude = std::max(std::min(
        (int)floor(log2(size) / 10),
        array_length-1), 0);

    int rounded_size = round((double)size / (exp2(magnitude*10)));

    return std::to_string(rounded_size) + " " + magnitude_prefixes[magnitude] + "B";
}

/* FileListModelColumns */

FileListModelColumns::FileListModelColumns():
    TreeModelColumnRecord(),
    col_full_path(),
    col_file_name(),
    col_size_str(),
    col_type_str(),
    col_last_modified()
{
    add(col_full_path);
    add(col_file_name);
    add(col_size_str);
    add(col_type_str);
    add(col_last_modified);
}

/* VFSFileChooserDialog */

VFSFileChooserDialog::VFSFileChooserDialog(
        FileSystem *vfs,
        bool open_for_writing):
    Dialog(),
    _vfs(vfs),
    _file_list_columns(),
    _file_list(ListStore::create(_file_list_columns)),
    _file_list_view(),
    _file_name_entry(),
    _open_for_writing(open_for_writing),
    _response_ok(false)
{
    _file_list_view.set_model(_file_list);

    set_position(WIN_POS_CENTER);
    set_default_size(400, 300);

    add_button(StockID("gtk-cancel"), 1);

    if (open_for_writing) {
        add_button(StockID("gtk-save"), 2);
    } else {
        add_button(StockID("gtk-open"), 2);
    }

    signal_response().connect(
        sigc::mem_fun(*this, &VFSFileChooserDialog::do_response));

    _file_list->set_sort_column(
        _file_list_columns.col_file_name,
        SORT_ASCENDING
        );
    _file_list_view.set_vexpand(true);
    _file_list_view.append_column(
        "File name", _file_list_columns.col_file_name);
    _file_list_view.append_column(
        "Size", _file_list_columns.col_size_str);
    _file_list_view.append_column(
        "Type", _file_list_columns.col_type_str);
    _file_list_view.append_column(
        "Last modified", _file_list_columns.col_last_modified);
    _file_list_view.signal_row_activated().connect(
        sigc::mem_fun(*this, &VFSFileChooserDialog::do_row_activated));

    ScrolledWindow *scroll = manage(new ScrolledWindow());
    scroll->add(_file_list_view);


    Box *content_area = get_content_area();
    if (open_for_writing) {
        _file_name_entry = manage(new Entry());
        content_area->pack_start(*_file_name_entry, false, true, 4);

        _file_name_entry->signal_activate().connect(
            sigc::mem_fun(
                *this,
                &VFSFileChooserDialog::do_file_name_entry_activate));

        _file_list_view.signal_cursor_changed().connect(
            sigc::mem_fun(
                *this,
                &VFSFileChooserDialog::do_cursor_changed));
    }
    content_area->pack_start(*scroll, true, true);

    show_all_children();

}

VFSFileChooserDialog::~VFSFileChooserDialog()
{

}

void VFSFileChooserDialog::add_file(
    const std::string &folder_path,
    const std::string &file_name)
{
    std::string full_path = join({folder_path, file_name});
    VFSStat stat;
    try {
        _vfs->stat(full_path, stat);
    } catch (const VFSIOError &err) {
        PyEngine::log->getChannel("io")->log(Warning)
            << err.what() << PyEngine::submit;
        PyEngine::log->getChannel("io")->log(Warning)
            << "File disappeared (stat failed, but returned from "
            << "listdir): " << full_path << PyEngine::submit;
        return;
    }

    if (!filter_file(stat)) {
        return;
    }

    std::string type_name;

    switch (get_type_from_stream(_vfs->open(full_path, OM_READ))) {
    case FT_LEVEL_COLLECTION:
    {
        type_name = "levels";
        break;
    }
    case FT_TILESET:
    {
        type_name = "tileset";
        break;
    }
    default:
        return;
    }

    ListStore::iterator iter = _file_list->append();
    ListStore::Row row = *iter;

    row[_file_list_columns.col_full_path] = full_path;
    row[_file_list_columns.col_file_name] = file_name;
    row[_file_list_columns.col_size_str] = format_file_size(
        stat.size);
    row[_file_list_columns.col_type_str] = type_name;

    time_t t = std::chrono::system_clock::to_time_t(stat.mtime);
    static char datestr[128];
    strftime(datestr, 128, "%c", std::localtime(&t));
    row[_file_list_columns.col_last_modified] = std::string(datestr);
}

void VFSFileChooserDialog::add_files_from_folder(
    const std::string &from_folder)
{
    std::vector<std::string> names;
    try {
        _vfs->listdir(from_folder, names);
    } catch (PyEngine::VFSFileNotFoundError &err) {
        // ignore that error
    }

    for (auto &name: names)
    {
        if ((name == ".") || (name == "..")) {
            continue;
        }
        add_file(from_folder, name);
    }
}

void VFSFileChooserDialog::clear()
{
    _file_list->clear();
    if (_open_for_writing) {
        _file_name_entry->set_text("");
    }
}

void VFSFileChooserDialog::do_cursor_changed()
{
    ListStore::iterator iter = get_selected_row();
    if (iter == _file_list->children().end()) {
        return;
    }

    TreeRow row = *iter;
    _file_name_entry->set_text(row[_file_list_columns.col_file_name]);
}

void VFSFileChooserDialog::do_file_name_entry_activate()
{
    std::string full_path = _file_name_entry->get_text();
    if (full_path == "") {
        return;
    }
    respond_with_file(join({_folder, full_path}));
}

void VFSFileChooserDialog::do_row_activated(
    const TreeModel::Path &path, TreeViewColumn *column)
{
    ListStore::iterator iter = _file_list->get_iter(path);
    assert(iter != _file_list->children().end());

    TreeRow row = *iter;
    respond_with_file(row[_file_list_columns.col_full_path]);
}

void VFSFileChooserDialog::do_response(int response_id)
{
    switch (response_id) {
    case 2:
    {
        if (_open_for_writing) {
            respond_with_file(_file_name_entry->get_text());
        } else {
            ListStore::iterator iter = get_selected_row();
            if (iter == _file_list->children().end()) {
                message_dlg(*this,
                    "Select a file",
                    "To continue, you have to select a file first.",
                    MESSAGE_ERROR,
                    BUTTONS_OK);
                return;
            }

            TreeRow row = *iter;
            respond_with_file(row[_file_list_columns.col_full_path]);
        }
        break;
    }
    case 1:
    default:
        _response_ok = false;
        hide();
        break;
    }
}

bool VFSFileChooserDialog::filter_file(const VFSStat &stat)
{
    return true;
}

ListStore::iterator VFSFileChooserDialog::get_selected_row()
{
    TreeModel::Path path;
    TreeViewColumn *dummy;
    _file_list_view.get_cursor(path, dummy);

    if (path.size() == 0) {
        return _file_list->children().end();
    }
    return _file_list->get_iter(path);
}

void VFSFileChooserDialog::respond_with_file(
    const std::string &full_path)
{
    if (_open_for_writing) {
        VFSStat stat;
        bool exists = false;
        try {
            _vfs->stat(full_path, stat);
            exists = true;
        } catch (const VFSIOError &err) {
        }
        if (exists) {
            if (message_dlg(*this,
                    "Overwrite?",
                    "Do you want to overwrite the file at `"+full_path+"'.",
                    MESSAGE_QUESTION,
                    BUTTONS_YES_NO) == GTK_RESPONSE_NO)
            {
                return;
            }
        }

    }

    _response_file_name = full_path;
    _response_ok = true;
    hide();
}

std::string VFSFileChooserDialog::select_file(
    const std::string &from_folder)
{
    _response_ok = false;
    _folder = from_folder;
    clear();
    add_files_from_folder(from_folder);
    run();
    if (_response_ok) {
        return _response_file_name;
    } else {
        return "";
    }
}

std::string VFSFileChooserDialog::select_file_multisource(
    const std::initializer_list<std::string> &sources)
{
    _response_ok = false;
    _folder = "ThisWasAnInvalidOperation";
    if (_open_for_writing) {
        throw std::logic_error("Cannot do multisource when opening for writing.");
    }

    clear();
    for (auto &source: sources) {
        add_files_from_folder(source);
    }
    run();
    if (_response_ok) {
        return _response_file_name;
    } else {
        return "";
    }
}
