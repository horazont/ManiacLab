/**********************************************************************
File name: Application.hpp
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
#ifndef _ML_APPLICATION_H
#define _ML_APPLICATION_H

#include <GL/glew.h>

#include <CEngine/IO/Log.hpp>
#include <CEngine/IO/Time.hpp>
#include <CEngine/VFS/FileSystem.hpp>
#include <CEngine/WindowInterface/Display.hpp>
#include <CEngine/WindowInterface/Window.hpp>
#include <CEngine/WindowInterface/EventLoop.hpp>
#include <CEngine/UI/Widgets/RootWidget.hpp>

#include "MainMenu.hpp"
#include "Playground.hpp"

class Application: public PyEngine::UI::RootWidget
{
public:
    Application(
        PyEngine::Display &dpy,
        const PyEngine::UI::coord_int_pair_t &dimensions = std::make_pair(800, 600),
        bool fullscreen = false,
        PyEngine::DisplayMode *display_mode = nullptr);
    virtual ~Application();

private:
    PyEngine::LogChannelHandle _log;
    PyEngine::Display &_dpy;
    PyEngine::WindowHandle _window;
    PyEngine::EventLoop *_loop;

    PyEngine::FileSystem _vfs;

    GLuint _cairo_tex;
    unsigned int _cairo_tex_w;
    unsigned int _cairo_tex_h;
    float _cairo_tex_s;
    float _cairo_tex_t;

    Mode *_current_mode;
    MainMenuMode _mode_main_menu;
    PlaygroundMode _mode_playground;

protected:
    void _recreate_cairo_surface(unsigned int width,
                                 unsigned int height) override;

protected:
    void set_mode(Mode *mode);

public:
    void switch_to_main_menu_mode();
    void switch_to_playground_mode();
    PyEngine::FileSystem &vfs();

public:
    void dispatch_wm_quit() override;
    bool ev_activate() override;
    bool ev_caret_motion(
        PyEngine::UI::CaretMotionDirection direction,
        PyEngine::UI::CaretMotionStep step,
        bool select) override;
    bool ev_deactivate() override;
    bool ev_key_down(PyEngine::Key::Key key,
                     PyEngine::UI::KeyModifiers modifiers) override;
    bool ev_key_up(PyEngine::Key::Key key,
                   PyEngine::UI::KeyModifiers modifiers) override;
    bool ev_mouse_click(
        int x, int y,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers,
        unsigned int nth) override;
    bool ev_mouse_down(
        int x, int y,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers) override;
    bool ev_mouse_enter() override;
    bool ev_mouse_leave() override;
    bool ev_mouse_move(
        int x, int y, int dx, int dy,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers) override;
    bool ev_mouse_up(
        int x, int y,
        PyEngine::UI::MouseButton button,
        PyEngine::UI::KeyModifiers modifiers) override;
    bool ev_resize() override;
    bool ev_scroll(int scrollx, int scrolly) override;
    bool ev_text_input(const char* buf) override;
    void frame_synced() override;
    void frame_unsynced(PyEngine::TimeFloat deltaT) override;
    void run(PyEngine::EventLoop *loop);
    void terminate();

};

#endif
