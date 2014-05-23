/**********************************************************************
File name: Mode.hpp
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
#ifndef _ML_MODE_H
#define _ML_MODE_H

#include <vector>

#include <CEngine/IO/Time.hpp>
#include <CEngine/UI/Widgets/WindowWidget.hpp>

class Application;

class Mode
{
public:
    Mode();
    virtual ~Mode();

protected:
    std::vector<PyEngine::UI::WidgetPtr> _desktop_widgets;
    std::vector<PyEngine::UI::Window*> _windows;
    Application *_root;

public:
    virtual void disable();
    virtual void enable(Application *root);
    virtual bool ev_activate();
    virtual bool ev_caret_motion(
        PyEngine::UI::CaretMotionDirection direction,
        PyEngine::UI::CaretMotionStep step,
        bool select);
    virtual bool ev_deactivate();
    virtual bool ev_key_down(PyEngine::Key::Key key,
                             PyEngine::UI::KeyModifiers modifiers);
    virtual bool ev_key_up(PyEngine::Key::Key key,
                           PyEngine::UI::KeyModifiers modifiers);
    virtual bool ev_mouse_click(
        int x, int y,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers,
        unsigned int nth);
    virtual bool ev_mouse_down(
        int x, int y,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers);
    virtual bool ev_mouse_enter();
    virtual bool ev_mouse_leave();
    virtual bool ev_mouse_move(
        int x, int y, int dx, int dy,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers);
    virtual bool ev_mouse_up(
        int x, int y,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers);
    virtual bool ev_resize();
    virtual bool ev_scroll(int scrollx, int scrolly);
    virtual bool ev_text_input(const char* buf);
    virtual bool ev_wm_quit();
    virtual void frame_synced();
    virtual void frame_unsynced(PyEngine::TimeFloat deltaT);

};

#endif
