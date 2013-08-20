/**********************************************************************
File name: GTKUtils.cpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#include "GTKUtils.hpp"

using namespace Glib;
using namespace Gtk;

void message_dlg(
        Window &parent,
        const std::string &primary_text,
        const std::string &secondary_text,
        MessageType message_type,
        ButtonsType buttons)
{
    MessageDialog dlg(parent, primary_text, false, message_type,
                      buttons, true);
    dlg.set_secondary_text(secondary_text);
    dlg.run();
}

sigc::connection bind_action(
    const RefPtr<Builder> &builder,
    const std::string &name,
    const Action::SlotActivate &slot,
    RefPtr<Action> *action_dest)
{
    RefPtr<Action> action;
    action = action.cast_dynamic(builder->get_object(name));
    if (action_dest) {
        *action_dest = action;
    }
    return action->signal_activate().connect(slot);
}
