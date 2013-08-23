#include "UniqueNameDialog.hpp"

#include "GTKUtils.hpp"

using namespace Gtk;
using namespace Glib;

/* UniqueNameDialog */

UniqueNameDialog::UniqueNameDialog(
        BaseObjectType *cobject,
        const RefPtr<Builder> &builder,
        Entry *unique_name):
    Dialog(cobject),
    _builder(builder),
    _unique_name(unique_name),
    _check_name_signal()
{
    _unique_name->signal_activate().connect(
        sigc::mem_fun(*this, &UniqueNameDialog::unique_name_activate));
    _unique_name->signal_changed().connect(
        sigc::mem_fun(*this, &UniqueNameDialog::unique_name_changed));
}

bool UniqueNameDialog::check_unique_name(const std::string &name)
{
    return _check_name_signal(name);
}

Entry *UniqueNameDialog::get_entry(
        const RefPtr<Builder> &builder,
        const std::string &name)
{
    Entry *entry = nullptr;
    builder->get_widget(name, entry);
    return entry;
}

void UniqueNameDialog::on_response(int response_id)
{
    switch (response_id) {
    case 2:
    {
        std::string unique_name = _unique_name->get_text();

        if (unique_name == "") {
            message_dlg(*this,
                "Invalid unique name",
                "The unique name must not be empty.",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }

        if (!check_unique_name(unique_name)) {
            message_dlg(*this,
                "Invalid unique name",
                "The unique name is not valid (not unique for "
                "example).",
                MESSAGE_ERROR,
                BUTTONS_OK);
            return;
        }

        response_ok();

        hide();
        break;
    }
    case 1:
    default:
        response_abort();

        hide();
        return;
    };
}

void UniqueNameDialog::response_abort()
{

}

void UniqueNameDialog::response_ok()
{

}

void UniqueNameDialog::unique_name_activate()
{
    on_response(2);
}

void UniqueNameDialog::unique_name_changed()
{
    if (_unique_name->get_text() == "") {
        _unique_name->set_icon_from_stock(StockID("gtk-dialog-warning"));
    } else {
        _unique_name->unset_icon();
    }
}
