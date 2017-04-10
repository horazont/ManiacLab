#ifndef APPLICATION_H
#define APPLICATION_H

#include "fixups.hpp"

#include <memory>

#include <QActionGroup>
#include <QMainWindow>
#include <QMetaObject>
#include <QMdiSubWindow>

#include "ffengine/io/filesystem.hpp"

#include "mode.hpp"
#include "openglscene.hpp"


namespace Ui {
class Application;
}

class MainMenu;
class InGame;


class Application : public QMainWindow
{
    Q_OBJECT
public:
    enum Mode {
        MAIN_MENU,
        INGAME,
    };

public:
    explicit Application(QWidget *parent = 0);
    ~Application();

private:
    Ui::Application *m_ui;

    std::unique_ptr<MainMenu> m_main_menu;
    std::unique_ptr<InGame> m_ingame;

    ApplicationMode *m_curr_mode;
    std::unordered_map<QMdiSubWindow*, QMetaObject::Connection> m_mdi_connections;

private:
    void enter_mode(ApplicationMode *mode);
    void initialise_modes();
    void mdi_window_closed();
    void mdi_window_opened();
    void subdialog_done(QMdiSubWindow *wnd);

protected:
    void resizeEvent(QResizeEvent *event) override;

public:
    void enter_mode(Mode mode);
    OpenGLScene &scene();
    void show_dialog(QDialog &window);
    void show_preferences_dialog();
    void show_widget_as_window(QWidget &window, Qt::WindowFlags flags = 0);
    void quit();

};

#endif // APPLICATION_HPP
