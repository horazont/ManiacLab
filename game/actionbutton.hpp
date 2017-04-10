/**********************************************************************
File name: actionbutton.hpp
This file is part of: SCC (working title)

LICENSE

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about SCC please e-mail one of the authors named in
the AUTHORS file.
**********************************************************************/
#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include "fixups.hpp"

#include <QAction>
#include <QPushButton>


class ActionButton: public QPushButton
{
    Q_OBJECT
public:
    explicit ActionButton(QWidget *parent = nullptr);

private:
    QAction *m_action_owner;

public:
    void set_action(QAction *action);

public slots:
    void update_button_status_from_action();

};


#endif
