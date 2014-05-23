#include "Playground.hpp"

#include "Application.hpp"

using namespace PyEngine;
using namespace PyEngine::UI;

/* PlaygroundScene */

PlaygroundScene::PlaygroundScene():
    Widget()
{
    _flags |= PyEngine::UI::WidgetFlag::FOCUSABLE;
}

bool PlaygroundScene::ev_key_up(Key::Key key,
                                KeyModifiers modifiers)
{
    switch (key) {
    default: {}
    }
    return false;
}

bool PlaygroundScene::is_element(const std::string &name) const
{
    return name == "scene";
}

/* PlaygroundMode */

PlaygroundMode::PlaygroundMode():
    Mode()
{
    _desktop_widgets.push_back(new PlaygroundScene());
}

void PlaygroundMode::enable(Application *root)
{
    Mode::enable(root);
    glClearColor(0, 0, 0, 1);
}

bool PlaygroundMode::ev_key_up(Key::Key key,
                               KeyModifiers modifiers)
{
    switch (key) {
    case Key::Escape:
    {
        _root->switch_to_main_menu_mode();
        return true;
    }
    default: {}
    }
    return false;
}

void PlaygroundMode::frame_unsynced(TimeFloat deltaT)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
