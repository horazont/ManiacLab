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

#include "RootWindow.hpp"

/* LevelCollectionEditor */

LevelCollectionEditor::LevelCollectionEditor(
        RootWindow *root,
        Gtk::Container *parent,
        LevelCollectionEditee *editee):
    Editor(root, parent),
    _editee(editee)
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

void LevelCollectionEditor::disable()
{

}

void LevelCollectionEditor::enable()
{

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
