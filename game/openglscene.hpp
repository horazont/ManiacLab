#ifndef OPENGLSCENE_HPP
#define OPENGLSCENE_HPP

#include "fixups.hpp"

#include <chrono>

#include <QOpenGLWidget>

#include "ffengine/render/scenegraph.hpp"
#include "ffengine/render/camera.hpp"
#include "ffengine/render/renderpass.hpp"

typedef std::chrono::steady_clock monoclock;

namespace Ui {
class OpenGLScene;
}

class OpenGLScene : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OpenGLScene(QWidget *parent = 0);
    ~OpenGLScene();

private:
    Ui::OpenGLScene *m_ui;

    monoclock::time_point m_t;

    ffe::RenderGraph *m_rendergraph;

    monoclock::time_point m_previous_t;
    double m_previous_fps;
    unsigned int m_frames;

signals:
    void advance(ffe::TimeInterval seconds);
    void after_gl_sync();
    void before_gl_sync();

protected:
    void advance_frame();
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

public:
    double fps();
    void setup_scene(ffe::RenderGraph *rendergraph);

};

#endif // OPENGLSCENE_H
