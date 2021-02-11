#ifndef INGAME_HPP
#define INGAME_HPP

#include "mode.hpp"

#include "logic/level.hpp"
#include "logic/player_object.hpp"
#include "logic/server.hpp"


namespace Ui {
class InGame;
}

struct InGameScene;

class InGame: public ApplicationMode
{
    Q_OBJECT

public:
    explicit InGame(Application &app, QWidget *parent = nullptr);
    ~InGame() override;

private:
    Ui::InGame *m_ui;

    ffe::TimeInterval m_time_buffer;

    QMetaObject::Connection m_advance_conn;
    QMetaObject::Connection m_after_gl_sync_conn;
    QMetaObject::Connection m_before_gl_sync_conn;

    PlayerController m_player_controller;

    PlayerObject *m_player;
    std::unique_ptr<Server> m_server;
    std::unique_ptr<InGameScene> m_scene;

    bool m_single_step;

    int m_mouse_pos_x;
    int m_mouse_pos_y;

protected:
    Vector2f widget_pos_to_level_pos(const float x, const float y);
    void update_probe(const CoordPair phy_probe_pos);

public slots:
    void advance(ffe::TimeInterval dt);
    void after_gl_sync();
    void before_gl_sync();

    // ApplicationMode interface
public:
    void activate(QWidget &parent) override;
    void deactivate() override;


    // QWidget interface
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // MAINMENU_HPP
