/**********************************************************************
File name: Mode.cpp
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
#include "Mode.hpp"

#include "Application.hpp"

using namespace PyEngine::UI;

/* Mode */

Mode::Mode():
    _desktop_widgets(),
    _windows(),
    _root(nullptr)
{

}

Mode::~Mode()
{
    if (_root) {
        disable();
    }
    for (auto widget: _desktop_widgets) {
        if (!widget->get_parent()) {
            delete widget;
        }
    }
    for (auto widget: _windows) {
        if (!widget->get_parent()) {
            delete widget;
        }
    }
}

void Mode::disable()
{
    for (auto widget: _desktop_widgets) {
        ParentPtr parent = widget->get_parent();
        if (parent) {
            parent->remove(widget);
        }
    }
    for (auto widget: _windows) {
        ParentPtr parent = widget->get_parent();
        if (parent) {
            parent->remove(widget);
        }
    }
}

void Mode::enable(Application *root)
{
    ParentPtr desktop = root->desktop_layer();
    for (auto widget: _desktop_widgets) {
        desktop->add(widget);
    }
}

void Mode::frame_synced()
{

}

void Mode::frame_unsynced(PyEngine::TimeFloat deltaT)
{

}
