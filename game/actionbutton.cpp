/**********************************************************************
File name: actionbutton.cpp
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
#include "actionbutton.hpp"


ActionButton::ActionButton(QWidget *parent):
    QPushButton(parent),
    m_action_owner(nullptr)
{

}

void ActionButton::set_action(QAction *action)
{
    if (m_action_owner) {
        disconnect(m_action_owner, SIGNAL(changed()),
                   this, SLOT(update_button_status_from_action()));
        disconnect(this, static_cast<void(ActionButton::*)(bool)>(&ActionButton::clicked),
                   m_action_owner, &QAction::triggered);
    }

    m_action_owner = action;

    if (m_action_owner) {
        update_button_status_from_action();
        connect(m_action_owner, SIGNAL(changed()),
                this, SLOT(update_button_status_from_action()));
        connect(this,  static_cast<void(ActionButton::*)(bool)>(&ActionButton::clicked),
                m_action_owner, &QAction::triggered);
    }
}

void ActionButton::update_button_status_from_action()
{
    setText(m_action_owner->text());
    setStatusTip(m_action_owner->statusTip());
    setToolTip(m_action_owner->toolTip());
    setIcon(m_action_owner->icon());
    setEnabled(m_action_owner->isEnabled());
    setCheckable(m_action_owner->isCheckable());
    setChecked(m_action_owner->isChecked());
}
