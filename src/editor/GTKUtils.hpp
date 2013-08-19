#ifndef _ML_GTK_UTILS_H
#define _ML_GTK_UTILS_H

#include <gtkmm.h>

void message_dlg(Gtk::Window &parent,
                 const std::string &primary_text,
                 const std::string &secondary_text,
                 Gtk::MessageType message_type,
                 Gtk::ButtonsType buttons);

#endif
