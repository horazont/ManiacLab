#include "mainmenu.hpp"
#include "ui_mainmenu.h"

#include "application.hpp"


MainMenu::MainMenu(Application &app, QWidget *parent) :
    ApplicationMode(app, parent),
    m_ui(new Ui::MainMenu)
{
    m_ui->setupUi(this);

    m_ui->btn_singleplayer->set_action(m_ui->action_mainmenu_singleplayer);
    m_ui->btn_map_editor->set_action(m_ui->action_mainmenu_map_editor);
    m_ui->btn_settings->set_action(m_ui->action_mainmenu_settings);
    m_ui->btn_quit->set_action(m_ui->action_mainmenu_quit);
}

MainMenu::~MainMenu()
{
    delete m_ui;
}

void MainMenu::on_action_mainmenu_quit_triggered()
{
    m_app.quit();
}

void MainMenu::on_action_mainmenu_settings_triggered()
{
    m_app.show_preferences_dialog();
}

void MainMenu::on_action_mainmenu_singleplayer_triggered()
{
    m_app.enter_mode(Application::INGAME);
}

void MainMenu::on_action_mainmenu_map_editor_triggered()
{
    m_app.enter_mode(Application::EDITOR);
}
