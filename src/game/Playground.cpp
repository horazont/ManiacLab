#include "Playground.hpp"

#include <CEngine/GL/GeometryBufferView.hpp>
#include <CEngine/Resources/Image.hpp>

#include "Application.hpp"

#include "logic/SafeWallObject.hpp"
#include "logic/RockObject.hpp"

using namespace PyEngine;
using namespace PyEngine::UI;


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
    _level()
{
    _desktop_widgets.push_back(new PlaygroundScene());
}

void PlaygroundMode::disable()
{
    _tilemats = nullptr;
    _fire_indicies = nullptr;
    _smoke_indicies = nullptr;
    _object_indicies = nullptr;
    _diffuse_indicies = nullptr;
    _emmission_indicies = nullptr;
    _atlas_geometry = nullptr;
    _object_geometry = nullptr;
    _level = nullptr;
    glDeleteTextures(1, &_debug_tex);
    glDeleteTextures(1, &_texatlas);
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
            GL_DYNAMIC_DRAW));
    _atlas_geometry = PyEngine::GL::GeometryBufferHandle(
        new PyEngine::GL::GeometryBuffer(
            PyEngine::GL::VertexFormatHandle(
                new PyEngine::GL::VertexFormat(3, 0, 2)),
            GL_DYNAMIC_DRAW));

    _object_indicies = PyEngine::GL::StreamIndexBufferHandle(
        new PyEngine::GL::StreamIndexBuffer());
    _fire_indicies = PyEngine::GL::StreamIndexBufferHandle(
        new PyEngine::GL::StreamIndexBuffer());
    _smoke_indicies = PyEngine::GL::StreamIndexBufferHandle(
        new PyEngine::GL::StreamIndexBuffer());
    _diffuse_indicies = PyEngine::GL::StreamIndexBufferHandle(
        new PyEngine::GL::StreamIndexBuffer());
    _emmission_indicies = PyEngine::GL::StreamIndexBufferHandle(
        new PyEngine::GL::StreamIndexBuffer());

    glGenTextures(1, &_texatlas);
    glBindTexture(GL_TEXTURE_2D, _texatlas);
    PyEngine::Resources::ImageHandle atlas =
        PyEngine::Resources::Image::PNGImage(
            _root->vfs().open(
                "/data/tileset/atlas.png",
                OM_READ));
    atlas->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    static const std::size_t texw = 256;
    static const std::size_t texh = 256;

    _tilemats = std::unique_ptr<TileMaterialManager>(new TileMaterialManager());

    Metatexture *diffuse = _tilemats->register_metatexture(
        "safewall0_diffuse",
        std::unique_ptr<Metatexture>(
            new SimpleMetatexture(
                _atlas_geometry,
                _diffuse_indicies,
                (64 + 0.5) / texw,
                (0 + 0.5) / texh,
                (128 - 0.5) / texw,
                (64 - 0.5) / texh,
                1.0,
                1.0)));
    Metatexture *emmission = _tilemats->register_metatexture(
        "safewall0_emmission",
        std::unique_ptr<Metatexture>(
            new SimpleMetatexture(
                _atlas_geometry,
                _emmission_indicies,
                (0 + 0.5) / texw,
                (64 + 0.5) / texh,
                (128 - 0.5) / texw,
                (192 - 0.5) / texh,
                3.0,
                3.0)));
    _tilemats->new_material(
        mat_safewall_standalone,
        diffuse,
        emmission);

    diffuse = _tilemats->register_metatexture(
        "player_diffuse",
        std::unique_ptr<Metatexture>(
            new SimpleMetatexture(
                _atlas_geometry,
                _diffuse_indicies,
                (128 + 0.5) / texw,
                (0 + 0.5) / texh,
                (192 - 0.5) / texw,
                (64 - 0.5) / texh,
                1.0,
                1.0)));
    _tilemats->new_material(
        mat_player,
        diffuse,
        nullptr);

    diffuse = _tilemats->register_metatexture(
        "rock_diffuse",
        std::unique_ptr<Metatexture>(
            new SimpleMetatexture(
                _atlas_geometry,
                _diffuse_indicies,
                (0 + 0.5) / texw,
                (0 + 0.5) / texh,
                (64 - 0.5) / texw,
                (64 - 0.5) / texh,
                1.0,
                1.0)));
    _tilemats->new_material(
        mat_rock,
        diffuse,
        nullptr);

    _player = new PlayerObject(_level.get());
    _level->place_player(
        _player,
        0, 0);
    _level->particles().clear();
    _player->setup_view(*_tilemats);

    GameObject *obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        2, 2);
    obj->setup_view(*_tilemats);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        1, 2);
    obj->setup_view(*_tilemats);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        0, 2);
    obj->setup_view(*_tilemats);

    obj = new RockObject(_level.get());
    _level->place_object(
        obj,
        2, 0);
    obj->setup_view(*_tilemats);

    glGenTextures(1, &_debug_tex);
    glBindTexture(GL_TEXTURE_2D, _debug_tex);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB8,
                 256, 256,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool PlaygroundMode::ev_key_down(Key::Key key,
                                 KeyModifiers modifiers)
{
    switch (key) {
    case Key::Up:
    case Key::Down:
    case Key::Left:
    case Key::Right:
    {
        if (modifiers == 0) {
            switch (key) {
            case Key::Up:
            {
                _player->acting = MOVE_UP;
                break;
            }
            case Key::Down:
            {
                _player->acting = MOVE_DOWN;
                break;
            }
            case Key::Right:
            {
                _player->acting = MOVE_RIGHT;
                break;
            }
            case Key::Left:
            {
                _player->acting = MOVE_LEFT;
                break;
            }
            default: {}
            }

        } else {
            _level->particles().spawn_generator(
                4,
                [this] (PhysicsParticle *part) {
                    part->type = ParticleType::FIRE;
                    part->x = _player->x + 1.0 + ((float)random() / RAND_MAX)*0.2 - 0.05;
                    part->y = _player->y + 0.5 + ((float)random() / RAND_MAX)*0.1 - 0.05;
                    part->vx = 8 + ((float)random() / RAND_MAX)*0.2 - 0.1;
                    part->vy = ((float)random() / RAND_MAX)*0.5 - 0.25;
                    part->ax = 0;
                    part->ay = 0;
                    part->lifetime = 1;
                });
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
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    LevelCell *cell = _level->get_cell(0, 0);
    for (int i = 0; i < level_width*level_height; i++) {
        GameObject *const obj = cell->here;
        if (!obj) {
            ++cell;
            continue;
        }

        if (obj->view) {
            obj->view->update(*obj, deltaT);
        }

        ++cell;
    }

    size_t i = 0;
    for (auto &part: _level->particles())
    {
        if (_particle_verticies.size() == i) {
            _particle_verticies.push_back(
                _object_geometry->allocateVertices(4));
        }

        PyEngine::GL::GeometryBufferView buffer(
            _object_geometry,
            _particle_verticies[i]);

        const PyEngine::GL::GLVertexFloat age = part->age / part->lifetime;
        const PyEngine::GL::GLVertexFloat size = age * 0.8 + 0.2;
        const PyEngine::GL::GLVertexFloat halfsize = size/2;
        const PyEngine::GL::GLVertexFloat x0 = part->x - halfsize;
        const PyEngine::GL::GLVertexFloat y0 = part->y - halfsize;

        std::array<PyEngine::GL::GLVertexFloat, 12> pos({
                x0, y0, 0,
                x0, y0+size, 0,
                x0+size, y0+size, 0,
                x0+size, y0, 0
                    });
        buffer.getPositionView()->set(&pos.front());

        switch (part->type)
        {
        case ParticleType::FIRE:
        {
            const PyEngine::GL::GLVertexFloat green = age * 0.5 + 0.5;
            const PyEngine::GL::GLVertexFloat red = age * 0.3 + 0.7;
            const PyEngine::GL::GLVertexFloat blue = age * 0.8 + 0.1;
            const PyEngine::GL::GLVertexFloat alpha = 1.0 - age;

            std::array<PyEngine::GL::GLVertexFloat, 16> colours({
                    red, green, blue, alpha,
                        red, green, blue, alpha,
                        red, green, blue, alpha,
                        red, green, blue, alpha
                        });
            buffer.getColourView()->set(&colours.front());

            _fire_indicies->add(_particle_verticies[i]);

            break;
        }
        case ParticleType::SMOKE:
        {
            const PyEngine::GL::GLVertexFloat green = 0.2;
            const PyEngine::GL::GLVertexFloat red = 0.2;
            const PyEngine::GL::GLVertexFloat blue = 0.2;
            const PyEngine::GL::GLVertexFloat alpha = 1.0 - age;

            std::array<PyEngine::GL::GLVertexFloat, 16> colours({
                    red, green, blue, alpha,
                        red, green, blue, alpha,
                        red, green, blue, alpha,
                        red, green, blue, alpha
                        });
            buffer.getColourView()->set(&colours.front());

            _smoke_indicies->add(_particle_verticies[i]);

            break;
        }
        }

        ++i;
    }


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const Rect &rect = _root->get_absolute_rect();
    glOrtho(rect.get_x(), rect.get_width(),
            rect.get_height(), rect.get_y(),
            -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // move view into center
    glTranslatef(rect.get_width()/2-32,
                 rect.get_height()/2-32,
                 0);
    glScalef(64, 64, 0);
    glTranslatef(-_player->x, -_player->y, 0);

    _atlas_geometry->bind();
    glBindTexture(GL_TEXTURE_2D, _texatlas);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _diffuse_indicies->drawUnbound(GL_QUADS);
    _diffuse_indicies->clear();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    _emmission_indicies->drawUnbound(GL_QUADS);
    _emmission_indicies->clear();
    glBindTexture(GL_TEXTURE_2D, 0);
    _atlas_geometry->unbind();

    _object_geometry->bind();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _object_indicies->drawUnbound(GL_QUADS);
    _object_indicies->clear();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _smoke_indicies->drawUnbound(GL_QUADS);
    _smoke_indicies->clear();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    _fire_indicies->drawUnbound(GL_QUADS);
    _fire_indicies->clear();
    _object_geometry->unbind();

    glBindTexture(GL_TEXTURE_2D, _debug_tex);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    _level->physics().wait_for();
    _level->physics().to_gl_texture(0.5, 1.5, false);

    glColor4f(1, 1, 1, 0.3);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(0, 250./256.);
    glVertex2f(0, level_height);
    glTexCoord2f(250./256., 250./256.);
    glVertex2f(level_width, level_height);
    glTexCoord2f(250./256., 0);
    glVertex2f(level_width, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}
