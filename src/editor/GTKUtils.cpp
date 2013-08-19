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

