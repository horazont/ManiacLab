#ifndef ML_INTRO_SCENE_H
#define ML_INTRO_SCENE_H

#include <ffengine/gl/resource.hpp>
#include <ffengine/render/renderpass.hpp>

#include <QSize>

class Quad: public ffe::scenegraph::Node
{
public:
    static const std::array<Vector4f, 4> vertices;

public:
    explicit Quad(ffe::Material &material);

public:
    ffe::Material &m_material;
    ffe::VBOAllocation m_vbo_alloc;
    ffe::IBOAllocation m_ibo_alloc;

    Matrix4f m_transform;

    // Node interface
public:
    void prepare(ffe::RenderContext &context) override;
    void render(ffe::RenderContext &context) override;
    void sync() override;

    inline Matrix4f &transform()
    {
        return m_transform;
    }

};

class IntroScene
{
protected:
    explicit IntroScene();

public:
    virtual ~IntroScene();

protected:
    ffe::GLResourceManager m_resources;
    ffe::WindowRenderTarget m_window;
    ffe::SceneGraph m_scenegraph;
    ffe::PerspectivalCamera m_camera;
    ffe::Scene m_scene;
    ffe::RenderGraph m_rendergraph;

    ffe::RenderPass &m_main_pass;

public:
    inline ffe::RenderGraph &rendergraph()
    {
        return m_rendergraph;
    }

    void set_fbo_id(GLuint framebuffer_object_id);

public:
    virtual void update_size(const QSize &new_size);

    /**
     * @brief Advance the scene by the specified time interval.
     *
     * Returns true if the scene is now over.
     *
     * @param dt Time slice to advance the scene by.
     * @return Whether the scene is over.
     */
    virtual bool advance(ffe::TimeInterval dt);

    virtual void sync();

};


class IntroScene1: public IntroScene
{
public:
    static constexpr float lightning_t = 1.5f;
    static constexpr std::size_t n_rain_layers = 3;

public:
    explicit IntroScene1();

private:
    ffe::ShaderProgram *m_lightcone_shader;
    ffe::ShaderProgram *m_lightning_shader;
    ffe::ShaderProgram *m_wall_shader;
    ffe::ShaderProgram *m_background_shader;

    std::array<ffe::Texture2D*, n_rain_layers> m_rain_textures;

    float m_t_abs;
    float m_t_movement;
    float m_t_lightning;
    bool m_lightning_triggered;

public:
    bool advance(ffe::TimeInterval dt) override;
    void sync() override;

};


class IntroScene2: public IntroScene
{
public:
    explicit IntroScene2();

private:
    Matrix4f m_rhead_transform_lhs;
    Matrix4f m_rhead_transform_rhs;
    Matrix4f *m_rhead_transform;
    Matrix4f m_lhead_transform_lhs;
    Matrix4f m_lhead_transform_rhs;
    Matrix4f *m_lhead_transform;

    Matrix4f m_rtail_transform_lhs;
    Matrix4f m_rtail_transform_rhs;
    Matrix4f *m_rtail_transform;

    Matrix4f m_ltail_transform_lhs;
    Matrix4f m_ltail_transform_rhs;
    Matrix4f *m_ltail_transform;

    float m_t_abs;
    float m_t_movement;

    // IntroScene interface
public:
    bool advance(ffe::TimeInterval dt) override;
};


#endif
