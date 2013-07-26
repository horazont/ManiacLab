#include <CEngine/IO/Log.hpp>
#include <CEngine/IO/Time.hpp>

#include <CEngine/WindowInterface/Display.hpp>
#include <CEngine/WindowInterface/Window.hpp>

#include <CEngine/UI/Widgets/RootWidget.hpp>

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

    unsigned int _cairo_tex;
    unsigned int _cairo_tex_w;
    unsigned int _cairo_tex_h;
    float _cairo_tex_s;
    float _cairo_tex_t;

protected:
    void _recreate_cairo_surface(unsigned int width,
                                 unsigned int height) override;

public:
    void frame_unsynced(PyEngine::TimeFloat deltaT) override;

};
