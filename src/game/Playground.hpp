#ifndef _ML_PLAYGROUND_H
#define _ML_PLAYGROUND_H

#include <CEngine/UI/Widgets/WidgetBase.hpp>
#include <CEngine/GL/IndexBuffer.hpp>

#include "Mode.hpp"

#include "logic/Level.hpp"

class PlaygroundScene: public PyEngine::UI::Widget
{
public:
    PlaygroundScene();

public:
    bool is_element(const std::string &name) const override;

};

class PlaygroundMode: public Mode
{
public:
    PlaygroundMode();

private:
    std::unique_ptr<Level> _level;
    PyEngine::GL::GeometryBufferHandle _object_geometry;
    PyEngine::GL::StreamIndexBufferHandle _object_indicies;

    ObjectInfo _player_object_info;
    GameObject *_player;

public:
    void disable() override;
    void enable(Application *root) override;
    bool ev_key_down(PyEngine::Key::Key key,
                     PyEngine::UI::KeyModifiers modifiers) override;
    bool ev_key_up(PyEngine::Key::Key key,
                   PyEngine::UI::KeyModifiers modifiers) override;
    void frame_synced() override;
    void frame_unsynced(PyEngine::TimeFloat deltaT) override;
};

#endif
