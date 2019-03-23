#include "application.hpp"
#include "ui_application.h"

#include <QColorDialog>
#include <QDialog>
#include <QMdiSubWindow>
#include <QResizeEvent>

#include "mainmenu.hpp"
#include "ingame.hpp"
#include "editor.hpp"


static io::Logger &logger = io::logging().get_logger("app");

Application::Application(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Application),
    m_main_menu(),
    m_ingame(),
    m_curr_mode(nullptr)
{
    m_ui->setupUi(this);
    m_ui->mdiArea->hide();

    initialise_modes();
}

Application::~Application()
{
    if (m_curr_mode) {
        enter_mode(nullptr);
    }
    delete m_ui;
}

void Application::enter_mode(ApplicationMode *mode)
{
    if (m_curr_mode) {
        logger.log(io::LOG_DEBUG, "deactivating previous mode");
        m_curr_mode->deactivate();
    }
    m_curr_mode = mode;
    if (m_curr_mode) {
        logger.log(io::LOG_DEBUG, "activating new mode");
        m_curr_mode->activate(*m_ui->modeParent);
    }
}

void Application::initialise_modes()
{
    m_main_menu = std::make_unique<MainMenu>(*this);
    m_ingame = std::make_unique<InGame>(*this);
    m_editor = std::make_unique<Editor>(*this);
}

void Application::subdialog_done(QMdiSubWindow *wnd)
{
    m_ui->mdiArea->removeSubWindow(wnd);
    auto iter = m_mdi_connections.find(wnd);
    wnd->widget()->disconnect(iter->second);
    m_mdi_connections.erase(iter);

    mdi_window_closed();
}

void Application::mdi_window_closed()
{
    if (m_ui->mdiArea->subWindowList().size() == 0) {
        m_ui->mdiArea->hide();
        if (m_curr_mode) {
            m_curr_mode->setFocus();
        }
    }
}

void Application::mdi_window_opened()
{
    m_ui->mdiArea->show();
}

void Application::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    QRect geometry = m_ui->centralwidget->geometry();
    m_ui->sceneWidget->setGeometry(geometry);
    m_ui->modeParent->setGeometry(geometry);
    if (m_curr_mode) {
        m_curr_mode->setGeometry(geometry);
    }
    m_ui->mdiArea->setGeometry(geometry);
}

void Application::enter_mode(Mode mode)
{
    switch (mode)
    {
    case MAIN_MENU:
    {
        enter_mode(m_main_menu.get());
        break;
    }
    case INGAME:
    {
        enter_mode(m_ingame.get());
        break;
    }
    case EDITOR:
    {
        enter_mode(m_editor.get());
        break;
    }
    }
}

OpenGLScene &Application::scene()
{
    return *m_ui->sceneWidget;
}

void Application::show_dialog(QDialog &window)
{
    const unsigned int w = window.width();
    const unsigned int h = window.height();

    const unsigned int x = std::round(float(width()) / 2. - float(w) / 2.);
    const unsigned int y = std::round(float(height()) / 2. - float(h) / 2.);

    QMdiSubWindow *wnd = m_ui->mdiArea->addSubWindow(&window, window.windowFlags());
    wnd->setGeometry(x, y, w, h);
    QMetaObject::Connection conn = connect(&window, &QDialog::finished,
                                           this, [this, wnd](int){ subdialog_done(wnd); });
    m_mdi_connections[wnd] = conn;
    window.open();
    mdi_window_opened();
}

void Application::show_preferences_dialog()
{
}

void Application::quit()
{
    close();
}
