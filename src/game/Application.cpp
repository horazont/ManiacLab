#include "Application.hpp"

#include <GL/glew.h>

#include <CEngine/GL/CairoUtils.hpp>

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
    _log(PyEngine::log->getChannel("maniaclab")),
    _dpy(dpy),
    _window(nullptr),
    _cairo_tex(0),
    _cairo_tex_w(0),
    _cairo_tex_h(0),
    _cairo_tex_s(0),
    _cairo_tex_t(0)
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

    std::unique_ptr<BackgroundRule> rule1(new BackgroundRule());
    rule1->set(FillPtr(new Colour(1, 0, 0, 1)));

    theme->add_rule(
        SelectorPtr(new Is("desktoplayer")),
        std::move(rule1)
    );

    set_theme(theme);

    glGenTextures(1, &_cairo_tex);
}

Application::~Application()
{
    glDeleteTextures(1, &_cairo_tex);
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

    if ((pot_w != _cairo_tex_w) || (pot_h != _cairo_tex_h)) {
        glBindTexture(GL_TEXTURE_2D, _cairo_tex);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            pot_w,
            pot_h,
            0,
            GL_RGBA,
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
    if (get_surface_dirty()) {
        glTexCairoSurfaceSubImage2D(
            GL_TEXTURE_2D, 0,
            0, 0,
            get_cairo_surface());
        raiseLastGLError();
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
