#ifndef INGAME_HPP
#define INGAME_HPP

#include "mode.hpp"

#include "logic/level.hpp"


namespace Ui {
class InGame;
}

class InGameScene;

class InGame: public ApplicationMode
{
    Q_OBJECT

public:
    explicit InGame(Application &app, QWidget *parent = 0);
    ~InGame() override;

private:
    Ui::InGame *m_ui;

    ffe::TimeInterval m_time_buffer;

    QMetaObject::Connection m_advance_conn;
    QMetaObject::Connection m_after_gl_sync_conn;
    QMetaObject::Connection m_before_gl_sync_conn;

    std::unique_ptr<Level> m_level;
    std::unique_ptr<InGameScene> m_scene;

public slots:
    void advance(ffe::TimeInterval dt);
    void after_gl_sync();
    void before_gl_sync();

    // ApplicationMode interface
public:
    void activate(QWidget &parent) override;
    void deactivate() override;

};

#endif // MAINMENU_HPP
