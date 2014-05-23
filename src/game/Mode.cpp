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

using namespace PyEngine;
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
    _root = nullptr;
}

void Mode::enable(Application *root)
{
    _root = root;
    ParentPtr desktop = root->desktop_layer();
    for (auto widget: _desktop_widgets) {
        desktop->add(widget);
    }
}

bool Mode::ev_activate()
{
    return false;
}

bool Mode::ev_caret_motion(
    CaretMotionDirection direction,
    CaretMotionStep step,
    bool select)
{
    return false;
}

bool Mode::ev_deactivate()
{
    return false;
}

bool Mode::ev_key_down(Key::Key key, KeyModifiers modifiers)
{
    return false;
}

bool Mode::ev_key_up(Key::Key key, KeyModifiers modifiers)
{
    return false;
}

bool Mode::ev_mouse_click(int x, int y, MouseButton button,
                          KeyModifiers modifiers, unsigned int nth)
{
    return false;
}

bool Mode::ev_mouse_down(int x, int y, MouseButton button,
                         KeyModifiers modifiers)
{
    return false;
}

bool Mode::ev_mouse_enter()
{
    return false;
}

bool Mode::ev_mouse_leave()
{
    return false;
}

bool Mode::ev_mouse_move(
    int x, int y,
    int dx, int dy,
    MouseButtons buttons,
    KeyModifiers modifiers)
{
    return false;
}

bool Mode::ev_mouse_up(
    int x, int y,
    MouseButton button,
    KeyModifiers modifiers)
{
    return false;
}

bool Mode::ev_resize()
{
    return false;
}

bool Mode::ev_scroll(int scrollx, int scrolly)
{
    return false;
}

bool Mode::ev_text_input(const char* buf)
{
    return false;
}

bool Mode::ev_wm_quit()
{
    return false;
}

void Mode::frame_synced()
{

}

void Mode::frame_unsynced(PyEngine::TimeFloat deltaT)
{

}
