#include "editor.hpp"

#include "ui_editor.h"

Editor::Editor(Application &app, QWidget *parent):
    ApplicationMode(app, parent),
    m_ui(new Ui::Editor())
{
    m_ui->setupUi(this);
}

Editor::~Editor()
{

}
