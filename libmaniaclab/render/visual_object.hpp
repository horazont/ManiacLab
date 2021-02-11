#ifndef ML_VISUAL_OBJECT_H
#define ML_VISUAL_OBJECT_H

#include <ffengine/render/scenegraph.hpp>
#include <ffengine/render/renderpass.hpp>

#include "logic/game_object.hpp"


class GameObjectVisual: public ffe::scenegraph::Node
{
public:
    using Neighbourhood = std::array<GameObject*, 4>;
public:
    explicit GameObjectVisual(ffe::ResourceManager &resources,
                              GameObject &obj,
                              const Neighbourhood &neigh);
    ~GameObjectVisual() override;

private:
    GameObject &m_obj;
    ffe::Material *m_material;
    ffe::IBOAllocation m_ibo_alloc;
    ffe::VBOAllocation m_vbo_alloc;

    unsigned int m_nframes;
    unsigned int m_nvariants;
    unsigned int m_variant;
    float m_frame_rate;
    float m_frame;
    float m_scale;

    bool m_static;

    // Node interface
public:
    void advance(ffe::TimeInterval seconds) override;
    void prepare(ffe::RenderContext &context) override;
    void render(ffe::RenderContext &context) override;
    void sync() override;

    inline bool is_static() const {
        return m_static;
    }

    inline void set_static(bool value) {
        m_static = value;
    }
};

class LevelView: public ffe::scenegraph::Group
{
private:
    class ObjectView: public ::ObjectView
    {
    public:
        ObjectView(LevelView &view, GameObjectVisual &visual);
        ~ObjectView() override;

    private:
        LevelView &m_view;
        GameObjectVisual &m_visual;

    };

public:
    explicit LevelView(
            ffe::ResourceManager &resources,
            Level &level,
            bool editor_mode);

private:
    ffe::ResourceManager &m_resources;
    Level &m_level;
    bool m_editor_mode;

    void object_created(Level &level, GameObject &obj);
    void object_deleted(GameObjectVisual &visual);
};

#endif
