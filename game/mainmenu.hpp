#ifndef MAINMENU_HPP
#define MAINMENU_HPP

#include "mode.hpp"

namespace Ui {
class MainMenu;
}

class MainMenu: public ApplicationMode
{
    Q_OBJECT

public:
    explicit MainMenu(Application &app, QWidget *parent = 0);
    ~MainMenu();

private slots:
    void on_action_mainmenu_quit_triggered();

    void on_action_mainmenu_settings_triggered();

    void on_action_mainmenu_singleplayer_triggered();

    void on_action_mainmenu_map_editor_triggered();

private:
    Ui::MainMenu *m_ui;

};

#endif // MAINMENU_HPP
