#ifndef _LEVEL_COLLECTION_EDITOR_H
#define _LEVEL_COLLECTION_EDITOR_H

#include "Editor.hpp"

#include "LevelCollectionEditee.hpp"

class LevelCollectionEditor: public Editor
{
public:
    LevelCollectionEditor(
        RootWindow *root,
        Gtk::Container *parent,
        LevelCollectionEditee *editee);

private:
    LevelCollectionEditee *_editee;

public:
    const std::string &get_name() const override;
    std::string get_tab_name() const override;
    std::string get_vfs_dirname() const override;
    void set_name(const std::string &name) override;

public:
    inline LevelCollectionEditee *editee() const {
        return _editee;
    };

public:
    void disable() override;
    void enable() override;
    void file_save(const PyEngine::StreamHandle &stream) override;

};

#endif
