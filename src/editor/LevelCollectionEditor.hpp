/**********************************************************************
File name: LevelCollectionEditor.hpp
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
#ifndef _LEVEL_COLLECTION_EDITOR_H
#define _LEVEL_COLLECTION_EDITOR_H

#include "Editor.hpp"

#include "LevelCollectionEditee.hpp"

class LevelCollectionEditor: public Editor
{
public:
    LevelCollectionEditor(
        RootWindow *root,
        Gtk::Container *parent,
        LevelCollectionEditee *editee);

private:
    LevelCollectionEditee *_editee;

public:
    const std::string &get_name() const override;
    std::string get_tab_name() const override;
    std::string get_vfs_dirname() const override;
    void set_name(const std::string &name) override;

public:
    inline LevelCollectionEditee *editee() const {
        return _editee;
    };

public:
    void disable() override;
    void enable() override;
    void file_save(const PyEngine::StreamHandle &stream) override;

};

#endif
