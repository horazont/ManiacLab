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

class TilesetEditor: public Editor
{
public:
    TilesetEditor(RootWindow *root,
                  Gtk::Container *parent,
                  TilesetEditee *editee);

private:
    TilesetEditee *_editee;
    Gtk::Box _main_box;

public:
    inline TilesetEditee *editee() {
        return _editee;
    };

public:
    void disable() override;
    void enable() override;

public:
    void file_save(const PyEngine::StreamHandle &stream) override;

};

#endif
