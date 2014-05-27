#ifndef _ML_PLAYGROUND_H
#define _ML_PLAYGROUND_H

#include <CEngine/UI/Widgets/WidgetBase.hpp>
#include <CEngine/GL/IndexBuffer.hpp>

#include "Mode.hpp"

#include "logic/Particles.hpp"
#include "logic/Level.hpp"
#include "logic/PlayerObject.hpp"

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
    static constexpr std::size_t texw = 1024;
    static constexpr std::size_t texh = 1024;

public:
    PlaygroundMode();

private:
    GLuint _debug_tex;
    GLuint _texatlas;

    std::unique_ptr<Level> _level;
    PyEngine::GL::GeometryBufferHandle _object_geometry;
    PyEngine::GL::GeometryBufferHandle _atlas_geometry;
    PyEngine::GL::StreamIndexBufferHandle _object_indicies;
    PyEngine::GL::StreamIndexBufferHandle _fire_indicies;
    PyEngine::GL::StreamIndexBufferHandle _smoke_indicies;
    PyEngine::GL::StreamIndexBufferHandle _diffuse_indicies;
    PyEngine::GL::StreamIndexBufferHandle _emission_indicies;
    std::vector<PyEngine::GL::VertexIndexListHandle> _particle_verticies;

    std::unique_ptr<TileMaterialManager> _tilemats;

    PlayerObject *_player;

protected:
    void setup_texture(
        const MaterialKey &key,
        const PyEngine::GL::StreamIndexBufferHandle &index_buffer,
        const float x0,
        const float y0,
        const float x1,
        const float y1,
        const float size);
    void setup_textures();
    void setup_materials();
    void setup_wall_type_materials(
        const std::string &prefix,
        const std::string &formprefix);

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
