#include "mode.hpp"

#include "application.hpp"


ApplicationMode::ApplicationMode(Application &app, QWidget *parent):
    QWidget(parent),
    m_app(app),
    m_gl_scene(nullptr)
{
}

ApplicationMode::~ApplicationMode()
{

}

void ApplicationMode::activate(QWidget &parent)
{
    m_gl_scene = &m_app.scene();
    setParent(&parent);
    setGeometry(0, 0, parent.width(), parent.height());
    setVisible(true);
}

void ApplicationMode::deactivate()
{
    setParent(nullptr);
    m_gl_scene = nullptr;
}
