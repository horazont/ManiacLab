#ifndef _ML_OPEN_IMAGE_H
#define _ML_OPEN_IMAGE_H

#include <gtkmm.h>

class OpenImage: public Gtk::FileChooserDialog
{
public:
    OpenImage(BaseObjectType *cobject,
               const Glib::RefPtr<Gtk::Builder> &builder);

private:
    bool _response_ok;
    std::string _filename;

protected:
    void on_file_activated();
    void on_response(int response_id) override;

public:
    Glib::RefPtr<Gdk::Pixbuf> select_image(
        bool auto_add_to_recent = true);

};

#endif
