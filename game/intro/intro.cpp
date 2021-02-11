#include "intro/intro.hpp"

#include <ffengine/gl/resource.hpp>
#include <ffengine/gl/fbo.hpp>
#include <ffengine/render/scenegraph.hpp>
#include <ffengine/render/camera.hpp>
#include <ffengine/render/fullscreenquad.hpp>
#include <ffengine/render/renderpass.hpp>

#include "materials.hpp"
#include "openglscene.hpp"
#include "intro/scene.hpp"

#include "ui_intro.h"

Intro::Intro(Application &app, QWidget *parent):
    ApplicationMode(app, parent),
    m_ui(new Ui::Intro()),
    m_scene_state(0)
{

}

Intro::~Intro()
{

}

void Intro::advance_scene_state()
{
    switch (m_scene_state) {
    case 0:
        m_scene = std::make_unique<IntroScene1>();
        m_scene_state++;
        break;
    case 1:
        m_scene = std::make_unique<IntroScene2>();
        m_scene_state++;
        break;
    case 2:
        m_scene_state = 0;
        advance_scene_state();
        break;
    default:;
    }
}

void Intro::advance(ffe::TimeInterval dt)
{
    if (m_scene && m_scene->advance(dt)) {
        /* scene is over, delete it */
        m_scene = nullptr;
    }
}

void Intro::after_gl_sync()
{

}

void Intro::before_gl_sync()
{
    if (!m_scene) {
        advance_scene_state();
    }

    /* nothing to render anymore! */
    if (!m_scene) {
        m_gl_scene->setup_scene(nullptr);
        return;
    }

    const QSize size = window()->size() * window()->devicePixelRatioF();
    m_scene->update_size(size);
    m_scene->set_fbo_id(m_gl_scene->defaultFramebufferObject());
    m_scene->sync();

    m_gl_scene->setup_scene(&m_scene->rendergraph());
    ffe::raise_last_gl_error();
}

void Intro::activate(QWidget &parent)
{
    std::cout << "activate" << std::endl;
    ApplicationMode::activate(parent);

    m_advance_conn = connect(
                m_gl_scene,
                &OpenGLScene::advance,
                this,
                &Intro::advance,
                Qt::DirectConnection
                );
    m_before_gl_sync_conn = connect(
                m_gl_scene,
                &OpenGLScene::before_gl_sync,
                this,
                &Intro::before_gl_sync,
                Qt::DirectConnection
                );
    m_after_gl_sync_conn = connect(
                m_gl_scene,
                &OpenGLScene::after_gl_sync,
                this,
                &Intro::after_gl_sync,
                Qt::DirectConnection
                );

    m_gl_scene->update();
}

void Intro::deactivate()
{
    disconnect(m_after_gl_sync_conn);
    disconnect(m_advance_conn);
    disconnect(m_before_gl_sync_conn);

    m_scene = nullptr;

    ApplicationMode::deactivate();
}
