#include "LevelCollectionEditor.hpp"

#include "RootWindow.hpp"

/* LevelCollectionEditor */

LevelCollectionEditor::LevelCollectionEditor(
        RootWindow *root,
        Gtk::Container *parent,
        LevelCollectionEditee *editee):
    Editor(root, parent),
    _editee(editee)
{

}

const std::string &LevelCollectionEditor::get_name() const
{
    return _editee->get_name();
}

std::string LevelCollectionEditor::get_tab_name() const
{
    std::string name = get_name();
    if (name == "") {
        name = "[unnamed]";
    }
    return "▦ "+name;
}

std::string LevelCollectionEditor::get_vfs_dirname() const
{
    return "/data/levels";
}

void LevelCollectionEditor::set_name(const std::string &name)
{
    _editee->set_name(name);
}

void LevelCollectionEditor::disable()
{

}

void LevelCollectionEditor::enable()
{

}

void LevelCollectionEditor::file_save(const PyEngine::StreamHandle &stream)
{
    _editee->editee()->save_to_stream(
        stream,
        std::bind(
            &RootWindow::get_tileset_name,
            _root,
            std::placeholders::_1));
}
