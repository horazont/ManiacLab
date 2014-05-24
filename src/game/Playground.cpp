#include "Playground.hpp"

#include <CEngine/GL/GeometryBufferView.hpp>

#include "Application.hpp"

using namespace PyEngine;
using namespace PyEngine::UI;

static const CellStamp player_stamp(
    {
        false, true, true, true, false,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        false, true, true, true, false,
    }
);

/* PlaygroundScene */

PlaygroundScene::PlaygroundScene():
    Widget()
{
    _flags |= PyEngine::UI::WidgetFlag::FOCUSABLE;
}

bool PlaygroundScene::is_element(const std::string &name) const
{
    return name == "scene";
}

/* PlaygroundMode */

PlaygroundMode::PlaygroundMode():
    Mode(),
    _level(),
    _player_object_info(player_stamp)
{
    _desktop_widgets.push_back(new PlaygroundScene());
}

void PlaygroundMode::disable()
{
    _object_geometry = nullptr;
    _level = nullptr;
    Mode::disable();
}

void PlaygroundMode::enable(Application *root)
{
    Mode::enable(root);
    glClearColor(0, 0, 0, 1);
    _level = std::unique_ptr<Level>(new Level(level_width, level_height));
    _object_geometry = PyEngine::GL::GeometryBufferHandle(
        new PyEngine::GL::GeometryBuffer(
            PyEngine::GL::VertexFormatHandle(
                new PyEngine::GL::VertexFormat(3, 4)),
            GL_DYNAMIC_DRAW)
    );
    _object_indicies = PyEngine::GL::StreamIndexBufferHandle(
        new PyEngine::GL::StreamIndexBuffer());

    _player = new GameObject(_player_object_info);
    _level->place_player(
        _player,
        0, 0);
}

bool PlaygroundMode::ev_key_down(Key::Key key,
                                 KeyModifiers modifiers)
{
    switch (key) {
    case Key::Up:
    {
        if (modifiers == 0) {
            _player->acting = MOVE_UP;
        }
        return true;
    }
    case Key::Down:
    {
        if (modifiers == 0) {
            _player->acting = MOVE_DOWN;
        }
        return true;
    }
    case Key::Left:
    {
        if (modifiers == 0) {
            _player->acting = MOVE_LEFT;
        }
        return true;
    }
    case Key::Right:
    {
        if (modifiers == 0) {
            _player->acting = MOVE_RIGHT;
        }
        return true;
    }
    default: {}
    }
    return false;
}

bool PlaygroundMode::ev_key_up(Key::Key key,
                               KeyModifiers modifiers)
{
    switch (key) {
    case Key::q:
    case Key::Escape:
    {
        _root->switch_to_main_menu_mode();
        return true;
    }
    default: {}
    }
    return false;
}

void PlaygroundMode::frame_synced()
{
    _level->update();
    _level->update();
}

void PlaygroundMode::frame_unsynced(TimeFloat deltaT)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LevelCell *cell = _level->get_cell(0, 0);
    for (int i = 0; i < level_width*level_height; i++) {
        if (!cell->here) {
            ++cell;
            continue;
        }

        GameObject *const obj = cell->here;

        if (obj->view.invalidated) {
            if (!obj->view.vertices) {
                obj->view.vertices = _object_geometry->allocateVertices(4);
            }
            PyEngine::GL::GeometryBufferView buffer(
                _object_geometry,
                obj->view.vertices);

            std::array<PyEngine::GL::GLVertexFloat, 12> pos({
                    (float)obj->x, (float)obj->y, 0,
                        (float)obj->x, (float)obj->y+1, 0,
                        (float)obj->x+1, (float)obj->y+1, 0,
                        (float)obj->x+1, (float)obj->y, 0
                        });
            buffer.getPositionView()->set(&pos.front());

            std::array<PyEngine::GL::GLVertexFloat, 16> colours({
                    1, 1, 1, 0,
                        1, 1, 0, 0,
                        1, 0, 1, 0,
                        0, 1, 1, 0
                        });
            buffer.getColourView()->set(&colours.front());

            // obj->view.invalidated = false;
        }

        _object_indicies->add(obj->view.vertices);

        ++cell;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const Rect &rect = _root->get_absolute_rect();
    glOrtho(rect.get_x(), rect.get_width(),
            rect.get_height(), rect.get_y(),
            -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(24, 24, 0);

    _object_geometry->bind();
    _object_indicies->bind();
    _object_indicies->draw(GL_QUADS);
    _object_indicies->clear();
    _object_indicies->unbind();
    _object_geometry->unbind();
}
