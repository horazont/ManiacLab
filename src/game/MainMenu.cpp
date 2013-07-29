/**********************************************************************
File name: MainMenu.cpp
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
#include "MainMenu.hpp"

#include <CEngine/UI/Widgets/RootWidget.hpp>
#include <CEngine/UI/Widgets/LabelWidget.hpp>
#include <CEngine/UI/Widgets/ButtonWidget.hpp>
#include <CEngine/UI/Widgets/SpaceWidget.hpp>

using namespace PyEngine::UI;

/* MainMenu */

MainMenu::MainMenu():
    AbstractVBox()
{
    LabelWidget *title = new LabelWidget("ManiacLab");
    title->css_classes().insert("title");
    add(title);
    LabelWidget *subtitle = new LabelWidget("◀ main menu ▶");
    subtitle->css_classes().insert("subtitle");
    add(subtitle);
    add(new Space());
    add(new Button("Select profile"));
    add(new Space());
    add(new Button("Continue playing"));
    add(new Space());

    Button *btn_quit = new Button("Quit");
    btn_quit->on_click().connect(sigc::mem_fun(this, &MainMenu::quit));
    add(btn_quit);
    add(new Space());

    absolute_rect().set_width(400);
    absolute_rect().set_height(6*30);
}

void MainMenu::quit(WidgetPtr sender)
{
    _root->dispatch_wm_quit();
}

bool MainMenu::is_element(const std::string &name) const
{
    return (name == "mainmenu");
}

/* MainMenuMode */

MainMenuMode::MainMenuMode():
    Mode()
{
    _desktop_widgets.push_back(new MainMenu());
}
