/**********************************************************************
File name: TileEditor.hpp
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
#ifndef _ML_TILE_EDITOR_H
#define _ML_TILE_EDITOR_H

#include <gtkmm.h>

#include "logic/PhysicsConfig.hpp"
#include "logic/Stamp.hpp"
#include "TilesetEditee.hpp"
#include "GTKUtils.hpp"

class RootWindow;

class TileEditor: public Gtk::Widget
{
public:
    static const unsigned int border_size = 3;
    static const unsigned int cell_size = 48;
    static const unsigned int editor_size =
        subdivision_count*cell_size +
        (subdivision_count-1)*border_size;

public:
    TileEditor(RootWindow *root, TilesetEditee *tileset);
    virtual ~TileEditor();

private:
    RootWindow *_root;
    TilesetEditee *_tileset;
    Glib::RefPtr<Gdk::Window> _gdk_window;
    SharedTile _tile;
    CellStamp _stamp;
    Gdk::RGBA _cell_colour;
    Gdk::RGBA _cell_colour_block;
    Gdk::RGBA _border_colour;
    Glib::RefPtr<Gdk::Pixbuf> _tile_pixbuf;

private:
    bool _painting;
    bool _paint_value;

protected:
    void finish_painting();
    bool get_stamp_value(uint_fast8_t x, uint_fast8_t y);
    bool hit_cell(gdouble mousex, gdouble mousey,
                  unsigned int &x, unsigned int &y);
    void set_stamp_value(uint_fast8_t x, uint_fast8_t y, bool value);
    void start_painting(bool paint_value);

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
    void flush_cell_stamp();
    void switch_tile(const SharedTile &tile);
    void update_cell_stamp();
};

#endif
