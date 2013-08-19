/**********************************************************************
File name: Editor.cpp
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
#include "Editor.hpp"

using namespace Gtk;
using namespace Glib;

/* Editor */

Editor::Editor(RootWindow *root, Container *parent):
    _root(root),
    _parent(parent),
    _filename_known(false),
    _filename()
{

}

Editor::~Editor()
{

}

void Editor::set_filename(const std::string &name)
{
    _filename_known = true;
    _filename = name;
}

void Editor::unset_filename()
{
    _filename_known = false;
}

void Editor::disable()
{

}

void Editor::enable()
{

}

void Editor::edit_copy()
{

}

void Editor::edit_cut()
{

}

void Editor::edit_delete()
{

}

void Editor::edit_paste()
{

}

void Editor::edit_select_all()
{

}

void Editor::file_save(const PyEngine::StreamHandle &stream)
{
    assert(false);
}
