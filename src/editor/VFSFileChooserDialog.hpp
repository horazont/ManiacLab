/**********************************************************************
File name: VFSFileChooserDialog.hpp
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
#ifndef _ML_VFS_FILE_CHOOSER_DIALOG_H
#define _ML_VFS_FILE_CHOOSER_DIALOG_H

#include <gtkmm.h>

#include <CEngine/VFS/FileSystem.hpp>

class FileListModelColumns: public Gtk::TreeModelColumnRecord
{
public:
    FileListModelColumns();

    Gtk::TreeModelColumn<std::string> col_full_path;
    Gtk::TreeModelColumn<Glib::ustring> col_file_name;
    Gtk::TreeModelColumn<Glib::ustring> col_size_str;
    Gtk::TreeModelColumn<Glib::ustring> col_type_str;
    Gtk::TreeModelColumn<Glib::ustring> col_last_modified;
};


class VFSFileChooserDialog: public Gtk::Dialog
{
public:
    VFSFileChooserDialog(
        PyEngine::FileSystem *filesystem,
        bool open_for_writing=false);
    virtual ~VFSFileChooserDialog();

private:
    PyEngine::FileSystem *_vfs;
    FileListModelColumns _file_list_columns;
    Glib::RefPtr<Gtk::ListStore> _file_list;
    Gtk::TreeView _file_list_view;
    Gtk::Entry *_file_name_entry;

protected:
    bool _open_for_writing;
    bool _response_ok;
    std::string _folder;
    std::string _response_file_name;

protected:
    virtual void add_file(
        const std::string &folder_path,
        const std::string &file_name);
    void add_files_from_folder(const std::string &from_folder);
    void clear();
    void do_cursor_changed();
    void do_file_name_entry_activate();
    void do_row_activated(
        const Gtk::TreeModel::Path &path,
        Gtk::TreeViewColumn *column);
    void do_response(int response_id);
    virtual bool filter_file(const PyEngine::VFSStat &stat);
    Gtk::ListStore::iterator get_selected_row();
    void respond_with_file(const std::string &full_path);

public:
    std::string select_file(const std::string &from_folder);
    std::string select_file_multisource(
        const std::initializer_list<std::string> &sources);

};

#endif
