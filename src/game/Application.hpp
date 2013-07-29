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
#include <CEngine/WindowInterface/Display.hpp>
#include <CEngine/WindowInterface/Window.hpp>
#include <CEngine/UI/Widgets/RootWidget.hpp>

#include "MainMenu.hpp"

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

    GLuint _cairo_tex;
    unsigned int _cairo_tex_w;
    unsigned int _cairo_tex_h;
    float _cairo_tex_s;
    float _cairo_tex_t;

    Mode *_current_mode;
    MainMenuMode _mode_main_menu;

protected:
    void _recreate_cairo_surface(unsigned int width,
                                 unsigned int height) override;

protected:
    void set_mode(Mode *mode);

public:
    void frame_unsynced(PyEngine::TimeFloat deltaT) override;

};

#endif
