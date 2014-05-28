#include "Playground.hpp"

#include <CEngine/GL/GeometryBufferView.hpp>
#include <CEngine/Resources/Image.hpp>

#include "Application.hpp"

#include "logic/WallObject.hpp"
#include "logic/RockObject.hpp"
#include "logic/BombObject.hpp"

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

void PlaygroundMode::setup_texture(
    const MaterialKey &key,
    const PyEngine::GL::StreamIndexBufferHandle &index_buffer,
    const float x0,
    const float y0,
    const float x1,
    const float y1,
    const float size)
{
    _tilemats->register_metatexture(
        key,
        std::unique_ptr<Metatexture>(
            new SimpleMetatexture(
                _atlas_geometry,
                index_buffer,
                x0 / texw,
                y0 / texh,
                x1 / texw,
                y1 / texh,
                size, size)));
}

void PlaygroundMode::setup_textures()
{
    setup_texture(
        "wall0_emission",
        _emission_indicies,
        192, 512,
        384, 704,
        3.0);
    setup_texture(
        "wall1_emission",
        _emission_indicies,
        384, 512,
        576, 704,
        3.0);
    setup_texture(
        "wall3_emission",
        _emission_indicies,
        0, 704,
        192, 896,
        3.0);
    setup_texture(
        "wall5_emission",
        _emission_indicies,
        192, 704,
        384, 896,
        3.0);
    setup_texture(
        "wall7_emission",
        _emission_indicies,
        384, 704,
        576, 896,
        3.0);
    setup_texture(
        "wallf_emission",
        _emission_indicies,
        0, 512,
        192, 704,
        3.0);

    setup_texture(
        "safewallsq0_diffuse",
        _diffuse_indicies,
        128, 0,
        192, 64,
        1.0);
    setup_texture(
        "safewallsq1_diffuse",
        _diffuse_indicies,
        192, 0,
        256, 64,
        1.0);
    setup_texture(
        "safewallsq3_diffuse",
        _diffuse_indicies,
        256, 0,
        320, 64,
        1.0);

    setup_texture(
        "safewall5_diffuse",
        _diffuse_indicies,
        320, 0,
        384, 64,
        1.0);
    setup_texture(
        "safewall7_diffuse",
        _diffuse_indicies,
        384, 0,
        448, 64,
        1.0);
    setup_texture(
        "safewallf_diffuse",
        _diffuse_indicies,
        448, 0,
        512, 64,
        1.0);

    setup_texture(
        "rock_diffuse",
        _diffuse_indicies,
        128, 64,
        192, 128,
        1.0);

    setup_texture(
        "player_diffuse",
        _diffuse_indicies,
        256, 192,
        384, 320,
        2.0);
    setup_texture(
        "player_emission",
        _emission_indicies,
        0, 896,
        128, 1024,
        2.0);

    setup_texture(
        "bomb_diffuse",
        _diffuse_indicies,
        384, 64,
        448, 128,
        1.0);

}

void PlaygroundMode::setup_materials()
{
    setup_wall_type_materials(
        "safewall",
        "rd");
    setup_wall_type_materials(
        "safewall",
        "sq");

    _tilemats->new_material(
        "player",
        _tilemats->get_metatexture("player_diffuse"),
        _tilemats->get_metatexture("player_emission"));

    _tilemats->new_material(
        "rock",
        _tilemats->get_metatexture("rock_diffuse"),
        nullptr);

    _tilemats->new_material(
        "bomb",
        _tilemats->get_metatexture("bomb_diffuse"),
        nullptr);

}

void PlaygroundMode::setup_wall_type_materials(
    const std::string &prefix,
    const std::string &formprefix)
{
    std::array<std::tuple<bool, char>, 6> info({
        std::make_tuple(true, '0'),
        std::make_tuple(true, '1'),
        std::make_tuple(true, '3'),
        std::make_tuple(false, '5'),
        std::make_tuple(false, '7'),
        std::make_tuple(false, 'f')});

    for (auto &item: info) {
        Metatexture *const diffuse = _tilemats->get_metatexture(
            prefix + (std::get<0>(item) ? formprefix : "") + std::get<1>(item)
            + "_diffuse");
        Metatexture *const emission = _tilemats->get_metatexture(
            std::string("wall") + std::get<1>(item) + "_emission");

        _tilemats->new_material(
            prefix + formprefix + std::get<1>(item),
            diffuse,
            emission);
    }

}

void PlaygroundMode::disable()
{
    _tilemats = nullptr;
    _fire_indicies = nullptr;
    _smoke_indicies = nullptr;
    _object_indicies = nullptr;
    _diffuse_indicies = nullptr;
    _emission_indicies = nullptr;
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
    _emission_indicies = PyEngine::GL::StreamIndexBufferHandle(
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

    _tilemats = std::unique_ptr<TileMaterialManager>(new TileMaterialManager());

    setup_textures();
    setup_materials();

    _player = new PlayerObject(_level.get());
    _level->place_player(
        _player,
        2, 15);
    _player->setup_view(*_tilemats);

    GameObject *obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        2, 12);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        1, 12);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        0, 12);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        1, 13);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        1, 14);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        0, 14);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        2, 14);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        1, 15);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        3, 14);
    obj = new SafeWallObject(_level.get());
    _level->place_object(
        obj,
        3, 15);

    _level->get_cell(0, 12)->here->setup_view(*_tilemats);
    _level->get_cell(1, 12)->here->setup_view(*_tilemats);
    _level->get_cell(2, 12)->here->setup_view(*_tilemats);
    _level->get_cell(1, 13)->here->setup_view(*_tilemats);
    _level->get_cell(1, 14)->here->setup_view(*_tilemats);
    _level->get_cell(0, 14)->here->setup_view(*_tilemats);
    _level->get_cell(2, 14)->here->setup_view(*_tilemats);
    _level->get_cell(1, 15)->here->setup_view(*_tilemats);
    _level->get_cell(3, 14)->here->setup_view(*_tilemats);
    _level->get_cell(3, 15)->here->setup_view(*_tilemats);

    obj = new BombObject(_level.get());
    _level->place_object(
        obj,
        3, 13);
    obj->setup_view(*_tilemats);

    obj = new BombObject(_level.get());
    _level->place_object(
        obj,
        3, 10);
    obj->setup_view(*_tilemats);

    // obj = new RockObject(_level.get());
    // _level->place_object(
    //     obj,
    //     2, 10);
    // obj->setup_view(*_tilemats);

    // obj = new RockObject(_level.get());
    // _level->place_object(
    //     obj,
    //     2, 9);
    // obj->setup_view(*_tilemats);

    for (CoordInt x = 0; x < 50; x++) {
        for (CoordInt y = 3; y < 6; y++) {
            if (x == 3) {
                obj = new BombObject(_level.get());
            } else {
                obj = new RockObject(_level.get());
            }
            _level->place_object(
                obj,
                x, y);
            obj->phi = ((float)rand() / RAND_MAX) * 2 * 3.14159;
            obj->setup_view(*_tilemats);
        }
    }

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
            const PyEngine::GL::GLVertexFloat alpha = (1.0 - age)*0.3;

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
        case ParticleType::FIRE_SECONDARY:
        {
            const PyEngine::GL::GLVertexFloat green = 0.2 + (1.0 - age) * 0.5;
            const PyEngine::GL::GLVertexFloat red = 0.2 + (1.0 - age) * 0.8;
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
    glTranslatef(rect.get_width()/2-31,
                 rect.get_height()/2-31,
                 0);
    glScalef(62, 62, 0);
    glTranslatef(-_player->x, -_player->y, 0);

    _atlas_geometry->bind();
    glBindTexture(GL_TEXTURE_2D, _texatlas);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _diffuse_indicies->drawUnbound(GL_QUADS);
    _diffuse_indicies->clear();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    _emission_indicies->drawUnbound(GL_QUADS);
    _emission_indicies->clear();
    glBindTexture(GL_TEXTURE_2D, 0);
    _atlas_geometry->unbind();

    _object_geometry->bind();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _smoke_indicies->drawUnbound(GL_QUADS);
    _smoke_indicies->clear();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _object_indicies->drawUnbound(GL_QUADS);
    _object_indicies->clear();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    _fire_indicies->drawUnbound(GL_QUADS);
    _fire_indicies->clear();
    _object_geometry->unbind();

    // glBindTexture(GL_TEXTURE_2D, _debug_tex);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_TEXTURE_2D);
    // glEnable(GL_BLEND);

    // _level->physics().wait_for();
    // _level->physics().to_gl_texture(0.0, 2.0, false);

    // glColor4f(1, 1, 1, 0.5);
    // glBegin(GL_QUADS);
    // glTexCoord2f(0, 0);
    // glVertex2f(0, 0);
    // glTexCoord2f(0, 250./256.);
    // glVertex2f(0, level_height);
    // glTexCoord2f(250./256., 250./256.);
    // glVertex2f(level_width, level_height);
    // glTexCoord2f(250./256., 0);
    // glVertex2f(level_width, 0);
    // glEnd();

    // glDisable(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 0);

}
