#ifndef _ML_PLAYGROUND_H
#define _ML_PLAYGROUND_H

#include <CEngine/UI/Widgets/WidgetBase.hpp>

#include "Mode.hpp"

class PlaygroundScene: public PyEngine::UI::Widget
{
public:
    PlaygroundScene();

public:
    bool ev_key_up(PyEngine::Key::Key key,
                   PyEngine::UI::KeyModifiers modifiers) override;
    bool is_element(const std::string &name) const override;

};

class PlaygroundMode: public Mode
{
public:
    PlaygroundMode();

public:
    void enable(Application *root);
    bool ev_key_up(PyEngine::Key::Key key,
                   PyEngine::UI::KeyModifiers modifiers) override;
    void frame_unsynced(PyEngine::TimeFloat deltaT);
};

#endif
