#ifndef _ML_LEVEL_EDITOR_H
#define _ML_LEVEL_EDITOR_H

#include <gtkmm.h>

#include "logic/PhysicsConfig.hpp"
#include "LevelCollectionEditee.hpp"

class RootWindow;

class LevelEditor: public Gtk::Widget
{
public:
    static constexpr unsigned width = level_width;
    static constexpr unsigned height = level_height;
    static constexpr unsigned cell_size = 30;
    static constexpr unsigned border_width = 2;
    static constexpr unsigned editor_width = width*(cell_size+border_width);
    static constexpr unsigned editor_height = height*(cell_size+border_width);

public:
    LevelEditor(RootWindow *root, LevelCollectionEditee *editee);
    virtual ~LevelEditor();

private:
    RootWindow *_root;
    LevelCollectionEditee *_collection;
    SharedLevel _level;
    Glib::RefPtr<Gdk::Window> _gdk_window;

    Gdk::RGBA _background;
    Gdk::RGBA _border_colour;
    Gdk::RGBA _border_colour_hover;

    LevelData::TileBinding _primary_brush, _secondary_brush;
    bool _painting;
    LevelData::TileBinding _paint_brush;

    bool _hover;
    std::pair<unsigned int, unsigned int> _hover_cell;

protected:
    void cell_to_client(unsigned int x, unsigned int y,
                        unsigned int &coordx, unsigned int &coordy);
    bool client_to_cell(int x, int y,
                        unsigned int &cellx, unsigned int &celly);
    void finish_painting();
    const SharedTile &get_cell(unsigned int x, unsigned int y);
    bool hit_cell(gdouble mousex, gdouble mousey,
                  unsigned int &x, unsigned int &y);
    void queue_draw_cell(unsigned int x, unsigned int y);
    void set_cell(unsigned int x, unsigned int y,
                  const LevelData::TileBinding &brush);
    void start_painting(const LevelData::TileBinding &brush);

protected:
    void get_preferred_height_for_width_vfunc(
        int width, int &minimum_height,
        int &natural_height) const override;
    void get_preferred_height_vfunc(
        int &minimum_height, int &natural_height) const override;
    void get_preferred_width_for_height_vfunc(
        int height, int &minimum_width,
        int &natural_width) const override;
    void get_preferred_width_vfunc(
        int &minimum_width, int &natural_width) const override;
    Gtk::SizeRequestMode get_request_mode_vfunc() const override;
    void on_size_allocate(Gtk::Allocation &allocation) override;
    void on_map() override;
    void on_unmap() override;
    void on_realize() override;
    void on_unrealize() override;
    bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    bool on_button_press_event(GdkEventButton *event) override;
    bool on_button_release_event(GdkEventButton *event) override;
    bool on_motion_notify_event(GdkEventMotion *event) override;

public:
    void set_primary_brush(const LevelData::TileBinding &brush);
    void set_secondary_brush(const LevelData::TileBinding &brush);
    void switch_level(const SharedLevel &level);

};

#endif
