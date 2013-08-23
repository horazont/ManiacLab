#ifndef _ML_UNIQUE_NAME_DIALOG_H
#define _ML_UNIQUE_NAME_DIALOG_H

#include <gtkmm.h>

class UniqueNameDialog: public Gtk::Dialog
{
public:
    typedef sigc::signal<bool, const std::string&> CheckSignal;

public:
    UniqueNameDialog(
        BaseObjectType *cobject,
        const Glib::RefPtr<Gtk::Builder> &builder,
        Gtk::Entry *unique_name);

protected:
    Glib::RefPtr<Gtk::Builder> _builder;
    Gtk::Entry *_unique_name;
    CheckSignal _check_name_signal;

protected:
    virtual bool check_unique_name(const std::string &name);
    static Gtk::Entry *get_entry(
        const Glib::RefPtr<Gtk::Builder> &builder,
        const std::string &name);
    void on_response(int response_id) override;
    virtual void response_abort();
    virtual void response_ok();
    void unique_name_activate();
    void unique_name_changed();

public:
    inline CheckSignal &signal_check_name() {
        return _check_name_signal;
    };

};

#endif
