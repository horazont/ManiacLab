/**********************************************************************
File name: MainMenu.hpp
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
#ifndef _ML_MAIN_MENU_H
#define _ML_MAIN_MENU_H

#include <CEngine/UI/Widgets/BoxWidget.hpp>

#include "Mode.hpp"

class MainMenu: public PyEngine::UI::AbstractVBox
{
public:
    MainMenu();

public:
    void map_editor(PyEngine::UI::WidgetPtr sender);
    void tileset_editor(PyEngine::UI::WidgetPtr sender);
    void quit(PyEngine::UI::WidgetPtr sender);

public:
    bool is_element(const std::string &name) const override;

};

class MainMenuMode: public Mode
{
public:
    MainMenuMode();

public:
    void enable(Application *root) override;
    bool ev_key_up(PyEngine::Key::Key key,
                   PyEngine::UI::KeyModifiers modifiers) override;
    bool ev_wm_quit() override;
    void frame_unsynced(PyEngine::TimeFloat deltaT) override;
};

#endif
