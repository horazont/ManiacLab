#ifndef ML_INTRO_H
#define ML_INTRO_H

#include "mode.hpp"

#include <memory>

#include <ffengine/common/types.hpp>


namespace Ui {
class Intro;
}

class IntroScene;

class Intro: public ApplicationMode
{
    Q_OBJECT
public:
    explicit Intro(Application &app, QWidget *parent = nullptr);
    ~Intro() override;

private:
    std::unique_ptr<Ui::Intro> m_ui;

    QMetaObject::Connection m_advance_conn;
    QMetaObject::Connection m_after_gl_sync_conn;
    QMetaObject::Connection m_before_gl_sync_conn;

    unsigned int m_scene_state;
    std::unique_ptr<IntroScene> m_scene;

private:
    void advance_scene_state();

public slots:
    void advance(ffe::TimeInterval dt);
    void after_gl_sync();
    void before_gl_sync();

    // ApplicationMode interface
public:
    void activate(QWidget &parent) override;
    void deactivate() override;

};


#endif
