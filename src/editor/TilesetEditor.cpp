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

using namespace Gtk;
using namespace Glib;

/* TilesetEditor */

TilesetEditor::TilesetEditor(
        RootWindow *root,
        Container *parent,
        TilesetEditee *editee):
    Editor(root, parent),
    _editee(editee)
{
    Label *test = manage(new Label("test"));
    _main_box.pack_start(*test, PACK_EXPAND_PADDING);

    parent->add(_main_box);
    parent->show_all_children();
}

void TilesetEditor::disable()
{
    _root->get_menu_tileset()->hide();
}

void TilesetEditor::enable()
{
    _root->get_menu_tileset()->show();
}

void TilesetEditor::file_save(const PyEngine::StreamHandle &stream)
{
    save_tileset_to_stream(*_editee->editee(), stream);
}
