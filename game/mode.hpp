#ifndef SCC_MODE_H
#define SCC_MODE_H

#include "fixups.hpp"

#include <QWidget>


class Application;
class OpenGLScene;

class ApplicationMode: public QWidget
{
    Q_OBJECT
public:
    ApplicationMode(Application &app, QWidget *parent = nullptr);
    virtual ~ApplicationMode();

protected:
    Application &m_app;
    OpenGLScene *m_gl_scene;

public:
    virtual void activate(QWidget &parent);
    virtual void deactivate();
};

#endif
