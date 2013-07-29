/**********************************************************************
File name: Application.cpp
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
#include "Application.hpp"

#include <iostream>

#include <CEngine/GL/CairoUtils.hpp>
#include <CEngine/UI/CSS/Rules.hpp>

using namespace PyEngine;
using namespace PyEngine::UI;
using namespace PyEngine::GL;

inline unsigned int make_pot(unsigned int v)
{
    // From http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    // Credit: Sean Anderson
    v -= 1;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
}

/* Application */

Application::Application(
        Display &dpy,
        const coord_dimensions_t &dimensions,
        bool fullscreen,
        DisplayMode *display_mode):
    RootWidget(),
    _log(PyEngine::log->getChannel("maniaclab")),
    _dpy(dpy),
    _window(nullptr),
    _cairo_tex(0),
    _cairo_tex_w(0),
    _cairo_tex_h(0),
    _cairo_tex_s(0),
    _cairo_tex_t(0),
    _current_mode(nullptr),
    _mode_main_menu()
{
    std::unique_ptr<DisplayMode> guard;
    std::vector<DisplayMode> modes(dpy.getDisplayModes());
    std::sort(modes.begin(), modes.end());

    if (display_mode == nullptr) {
        if (modes.size() == 0) {
            throw std::runtime_error("No display modes found.");
        }

        display_mode = &modes.back();
    }

    DisplayMode *candidate = new DisplayMode(*display_mode);
    guard = std::unique_ptr<DisplayMode>(candidate);

    candidate->samples = 0;
    if (std::find(modes.begin(), modes.end(), *candidate) == modes.end()) {
        _log->log(Debug) << "No display mode " << *candidate << " available..." << submit;
        candidate->samples = display_mode->samples;
    }

    candidate->stencilBits = 0;
    if (std::find(modes.begin(), modes.end(), *candidate) == modes.end()) {
        _log->log(Debug) << "No display mode " << *candidate << " available..." << submit;
        candidate->stencilBits = display_mode->stencilBits;
    }
    display_mode = candidate;

    /* TODO: vfs */

    _log->log(Information) << "Creating context with display mode "
        << *display_mode << submit;

    _window = _dpy.createWindow(
        *display_mode,
        dimensions.first,
        dimensions.second,
        fullscreen);

    const auto& screens = dpy.getScreens();
    if (fullscreen) {
        _window->setFullscreen(
            screens[0].index,
            screens[0].index,
            screens[0].index,
            screens[0].index);
    } else {
        _window->setWindowed(
            screens[0].index,
            dimensions.first,
            dimensions.second);
    }

    _window->switchTo();
    _window->setTitle("ManiacLab");
    _window->initializeGLEW();

    glClearColor(0, 0.05, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _window->flip();
    absolute_rect() = Rect(0, 0, dimensions.first, dimensions.second);
    realign();

    ThemePtr theme(new Theme());

    theme->add_rule(
        SelectorPtr(new Is("root")),
        rule_group({
            simple_rule<FontFamilyRule>("Cantarell"),
            simple_rule<FontSizeRule>(11),
            simple_rule<TextColourRule>(FillPtr(new Colour(1, 1, 1, 1)))
        })
    );

    theme->add_rule(
        SelectorPtr(new Is("button")),
        rule_group({
            simple_rule<VerticalAlignRule>(CSS_VERTICAL_ALIGN_MIDDLE),
            simple_rule<TextAlignRule>(PANGO_ALIGN_CENTER)
        })
    );

    /* main menu */

    SelectorPtr mainmenu(new Is("mainmenu"));

    {
        std::unique_ptr<RuleGroup> group = rule_group({
            simple_rule<HeightRule>(600),
            simple_rule<WidthRule>(400),
            simple_rule<BackgroundRule>(FillPtr(new Colour(0, 0, 0, 0.7))),
            simple_rule<FontWeightRule>(PANGO_WEIGHT_BOLD),
            simple_rule<TextAlignRule>(PANGO_ALIGN_CENTER)
        });

        MarginRule *margin = new MarginRule();
        margin->set_all(Auto());
        group->add(RulePtr(margin));

        PaddingRule *padding = new PaddingRule();
        padding->set_left(32);
        padding->set_right(32);
        padding->set_bottom(16);
        group->add(RulePtr(padding));

        BorderRule *border = new BorderRule();
        FillPtr border_fill(new Colour(0.5, 0.5, 0.5, 1.0));
        border->set_left_width(3);
        border->set_left_fill(border_fill);
        border->set_right_width(3);
        border->set_right_fill(border_fill);
        group->add(RulePtr(border));

        theme->add_rule(
            mainmenu,
            std::move(group)
        );

    }

    theme->add_rule(
        SelectorPtr(new ChildOf(mainmenu,
                                SelectorPtr(new Is("label",
                                            SelectorPtr(new HasCSSClass("title")))))),
        rule_group({
            simple_rule<FontSizeRule>(36),
            simple_rule<HeightRule>(48)
        })
    );

    theme->add_rule(
        SelectorPtr(new ChildOf(mainmenu,
                                SelectorPtr(new Is("label",
                                            SelectorPtr(new HasCSSClass("subtitle")))))),
        rule_group({
            simple_rule<FontSizeRule>(16),
            simple_rule<HeightRule>(20)
        })
    );

    {
        std::unique_ptr<RuleGroup> group = rule_group({
            simple_rule<TextColourRule>(FillPtr(new Colour(1, 1, 1, 0.8))),
            simple_rule<FontSizeRule>(20),
            simple_rule<FlexRule>(2)
        });

        MarginRule *margin = new MarginRule();
        margin->set_left(16);
        margin->set_right(16);
        group->add(std::move(RulePtr(margin)));

        PaddingRule *padding = new PaddingRule();
        padding->set_all(10);
        group->add(std::move(RulePtr(padding)));

        BorderRule *border = new BorderRule();
        border->set_all_fills(FillPtr(new Colour(0, 0.5, 0.7, 1.0)));
        border->set_all_widths(1);
        group->add(std::move(RulePtr(border)));

        theme->add_rule(
            SelectorPtr(new ChildOf(mainmenu,
                                    SelectorPtr(new Is("button")))),
            std::move(group)
        );
    }

    theme->add_rule(
        SelectorPtr(new ChildOf(mainmenu,
                                SelectorPtr(new Is("space")))),
        rule_group({
            simple_rule<FlexRule>(1)
        })
    );

    theme->add_rule(
        SelectorPtr(new ChildOf(mainmenu,
                                SelectorPtr(new Is("button",
                                            SelectorPtr(new State(Hovered)))))),
        rule_group({
            simple_rule<BackgroundRule>(FillPtr(new Colour(0, 0.5, 0.8, 0.3))),
            simple_rule<TextColourRule>(Inherit)
        })
    );


    set_theme(theme);

    set_mode(&_mode_main_menu);
}

Application::~Application()
{
    if (_cairo_tex != 0) {
        glDeleteTextures(1, &_cairo_tex);
    }
}

void Application::_recreate_cairo_surface(
        unsigned int width,
        unsigned int height)
{
    RootWidget::_recreate_cairo_surface(width, height);

    unsigned int pot_w = make_pot(width);
    unsigned int pot_h = make_pot(height);

    _cairo_tex_s = (float)width / pot_w;
    _cairo_tex_t = (float)height / pot_h;

    if (_cairo_tex == 0) {
        glGenTextures(1, &_cairo_tex);
    }

    if ((pot_w != _cairo_tex_w) || (pot_h != _cairo_tex_h)) {
        glBindTexture(GL_TEXTURE_2D, _cairo_tex);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            pot_w,
            pot_h,
            0,
            GL_LUMINANCE,
            GL_UNSIGNED_BYTE,
            nullptr);
        raiseLastGLError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        raiseLastGLError();
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
        raiseLastGLError();
        glBindTexture(GL_TEXTURE_2D, 0);

        _cairo_tex_w = pot_w;
        _cairo_tex_h = pot_h;
    }
}

void Application::set_mode(Mode *mode)
{
    if (mode == _current_mode) {
        return;
    }

    if (_current_mode) {
        _current_mode->disable();
    }
    _current_mode = mode;
    if (_current_mode) {
        _current_mode->enable(this);
    }
}

void Application::frame_unsynced(TimeFloat deltaT)
{
    _window->switchTo();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Rect &rect = absolute_rect();
    glViewport(0, 0, rect.get_width(), rect.get_height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(rect.get_x(), rect.get_width(), rect.get_height(), rect.get_y(), -1, 1);
    glMatrixMode(GL_MODELVIEW);

    render();

    glBindTexture(GL_TEXTURE_2D, _cairo_tex);
    if (get_surface_dirty() || true) {
        glTexCairoSurfaceSubImage2D(
            GL_TEXTURE_2D, 0,
            0, 0,
            get_cairo_surface());
    }

    float s = _cairo_tex_s,
          t = _cairo_tex_t;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(0, 0);

        glTexCoord2f(0, t);
        glVertex2f(0, rect.get_height());

        glTexCoord2f(s, t);
        glVertex2f(rect.get_width(), rect.get_height());

        glTexCoord2f(s, 0);
        glVertex2f(rect.get_width(), 0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    _window->flip();
}
