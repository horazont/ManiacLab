#include "fixups.hpp"

#include "openglscene.hpp"
#include "ui_openglscene.h"

#include "ffengine/gl/debug.hpp"

#include <QWindow>

static io::Logger &logger = io::logging().get_logger("app.glscene");

OpenGLScene::OpenGLScene(QWidget *parent) :
    QOpenGLWidget(parent),
    m_ui(new Ui::OpenGLScene),
    m_t(monoclock::now()),
    m_rendergraph(nullptr),
    m_previous_t(m_t),
    m_frames(0)
{
    m_ui->setupUi(this);
}

OpenGLScene::~OpenGLScene()
{
    delete m_ui;
}

void OpenGLScene::advance_frame()
{
    monoclock::time_point t_now = monoclock::now();
    auto seconds_passed =
            std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1, 1> > >(t_now-m_t).count();
    emit advance(seconds_passed);
    m_t = t_now;
}

void OpenGLScene::initializeGL()
{
    QSurfaceFormat format = this->format();

    logger.log(io::LOG_INFO)
            << "created context, version "
            << format.majorVersion()
            << "."
            << format.minorVersion()
            << io::submit;

    io::LogLevel context_info_level = io::LOG_DEBUG;
    if (format.profile() != QSurfaceFormat::CoreProfile ||
            format.majorVersion() != 3 ||
            format.depthBufferSize() == 0)
    {
        context_info_level = io::LOG_WARNING;
        logger.log(io::LOG_EXCEPTION)
                << "Could not create Core-profile OpenGL 3+ context with depth buffer"
                << io::submit;
    } else {
        logger.log(io::LOG_DEBUG,
                          "context deemed appropriate, continuing...");
    }

    logger.log(context_info_level)
            << "  renderable  : "
            << (format.renderableType() == QSurfaceFormat::OpenGL
                ? "OpenGL"
                : format.renderableType() == QSurfaceFormat::OpenGLES
                ? "OpenGL ES"
                : format.renderableType() == QSurfaceFormat::OpenVG
                ? "OpenVG (software?)"
                : "unknown")
            << io::submit;
    logger.log(context_info_level)
            << "  rgba        : "
            << format.redBufferSize() << " "
            << format.greenBufferSize() << " "
            << format.blueBufferSize() << " "
            << format.alphaBufferSize()
            << io::submit;
    logger.log(context_info_level)
            << "  stencil     : "
            << format.stencilBufferSize()
            << io::submit;
    logger.log(context_info_level)
            << "  depth       : " << format.depthBufferSize()
            << io::submit;
    logger.log(context_info_level)
            << "  multisamples: " << format.samples()
            << io::submit;
    logger.log(context_info_level)
            << "  profile     : "
            << (format.profile() == QSurfaceFormat::CoreProfile
                ? "core"
                : "compatibility")
            << io::submit;

    logger.log(io::LOG_DEBUG)
            << "OpenGL limits:" << io::submit;
    logger.log(io::LOG_DEBUG)
            << "  GL_MAX_ARRAY_TEXTURE_LAYERS = "
            << ffe::gl_get_integer(GL_MAX_ARRAY_TEXTURE_LAYERS)
            << io::submit;

    logger.log(io::LOG_DEBUG)
            << "QOpenGLScene uses FBO #"
            << defaultFramebufferObject()
            << io::submit;

    const unsigned int gl_version = epoxy_gl_version();
    const unsigned int gl_major = gl_version / 10;
    const unsigned int gl_minor = gl_version % 10;
    logger.log(io::LOG_INFO)
            << "libepoxy reports OpenGL version " << gl_major << "." << gl_minor
            << io::submit;

    glEnable(GL_DEBUG_OUTPUT);
    ffe::send_gl_debug_to_logger(io::logging().get_logger("gl.debug"));
}

void OpenGLScene::resizeGL(int, int)
{

}

void OpenGLScene::paintGL()
{
    glGetError();
    emit before_gl_sync();
    if (m_rendergraph) {
        m_rendergraph->prepare();
    }
    emit after_gl_sync();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

#ifdef TIMELOG_QUICKGLSCENE
    timelog_clock::time_point t0 = timelog_clock::now();
    timelog_clock::time_point t_render;
#endif

    if (m_rendergraph) {
        glGetError();
        m_rendergraph->render();
        // make sure that no vertex array is bound anymore so that no incorrect
        // state changes happen
        glBindVertexArray(0);
        ffe::raise_last_gl_error();
    } else {
        logger.log(io::LOG_WARNING, "nothing to draw");
    }

#ifdef TIMELOG_QUICKGLSCENE
    t_render = timelog_clock::now();
    logger.logf(io::LOG_DEBUG, "quickglscene: paint %.2f ms",
                TIMELOG_ms(t_render - t0));
#endif
    m_frames += 1;
    {
        monoclock::time_point t_now = monoclock::now();
        auto seconds_passed =
                std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1, 1> > >(t_now-m_previous_t).count();
        if (seconds_passed > 1.f) {
            m_previous_fps = m_frames / seconds_passed;
            logger.logf(io::LOG_DEBUG, "%.0f FPS", m_previous_fps);
            m_previous_t = t_now;
            m_frames = 0;
        }
    }

    if (m_rendergraph) {
        update();
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    advance_frame();
}

double OpenGLScene::fps()
{
    return m_previous_fps;
}

void OpenGLScene::setup_scene(ffe::RenderGraph *rendergraph)
{
    m_rendergraph = rendergraph;
    update();
}

