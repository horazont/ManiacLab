/**********************************************************************
File name: TileEditor.cpp
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
#include "TileEditor.hpp"

#include "RootWindow.hpp"
#include "TileOperations.hpp"

using namespace Gtk;

/* TileEditor */

TileEditor::TileEditor(RootWindow *root, TilesetEditee *tileset):
    Glib::ObjectBase("tileeditor"),
    Gtk::Widget(),
    _root(root),
    _tileset(tileset),
    _tile(nullptr),
    _cell_colour(Gdk::RGBA("black")),
    _cell_colour_block(Gdk::RGBA("red")),
    _border_colour(Gdk::RGBA("white")),
    _tile_pixbuf()
{
    set_has_window(true);
    set_sensitive(false);

    _cell_colour.set_alpha(0.25);
    _cell_colour_block.set_alpha(0.25);
}

TileEditor::~TileEditor()
{

}

void TileEditor::finish_painting()
{
    _root->execute_operation(OperationPtr(new OpSetTileCellStamp(
        _tileset, _tile, _stamp)));
    _painting = false;
}

bool TileEditor::get_stamp_value(uint_fast8_t x, uint_fast8_t y)
{
    return _stamp.get_blocking(x, y);
}

bool TileEditor::hit_cell(gdouble mousex, gdouble mousey,
                          unsigned int &x, unsigned int &y)
{
    int mx = round(mousex);
    int my = round(mousey);

    if ((mx < 0) || (mx >= (int)editor_size) ||
        (my < 0) || (my >= (int)editor_size))
    {
        return false;
    }

    const int total_cell_size = cell_size + border_size;
    int xcell = mx / total_cell_size;
    int ycell = my / total_cell_size;

    if ((mx - xcell * total_cell_size) >= (int)cell_size) {
        return false;
    }
    if ((my - ycell * total_cell_size) >= (int)cell_size) {
        return false;
    }

    x = xcell;
    y = ycell;

    return true;
}

void TileEditor::set_stamp_value(uint_fast8_t x, uint_fast8_t y,
                                 bool value)
{
    _stamp[x + y*subdivision_count].type = (value ? CELL_BLOCK : CELL_CLEAR);
    queue_draw_area(0, 0, editor_size, editor_size);
}

void TileEditor::start_painting(bool paint_value)
{
    _painting = true;
    _paint_value = paint_value;
}

void TileEditor::get_preferred_height_for_width_vfunc(
    int width, int &minimum_height, int &natural_height) const
{
    return get_preferred_height_vfunc(minimum_height, natural_height);
}

void TileEditor::get_preferred_height_vfunc(
    int &minimum_height, int &natural_height) const
{
    minimum_height = editor_size;
    natural_height = editor_size;
}

void TileEditor::get_preferred_width_for_height_vfunc(
    int height, int &minimum_width, int &natural_width) const
{
    return get_preferred_width_vfunc(minimum_width, natural_width);
}

void TileEditor::get_preferred_width_vfunc(
    int &minimum_width, int &natural_width) const
{
    minimum_width = editor_size;
    natural_width = editor_size;
}

SizeRequestMode TileEditor::get_request_mode_vfunc() const
{
    return Widget::get_request_mode_vfunc();
}

void TileEditor::on_size_allocate(Allocation &allocation)
{
    set_allocation(allocation);
    if (_gdk_window) {
        _gdk_window->move_resize(
            allocation.get_x(),
            allocation.get_y(),
            allocation.get_width(),
            allocation.get_height());
    }
}

void TileEditor::on_map()
{
    Widget::on_map();
}

void TileEditor::on_unmap()
{
    Widget::on_unmap();
}

void TileEditor::on_realize()
{
    set_realized();

    if (!_gdk_window) {
        GdkWindowAttr attributes;
        memset(&attributes, 0, sizeof(attributes));

        Allocation allocation = get_allocation();
        attributes.x = allocation.get_x();
        attributes.y = allocation.get_y();
        attributes.width = allocation.get_width();
        attributes.height = allocation.get_height();

        attributes.event_mask = get_events()
                              | Gdk::EXPOSURE_MASK
                              | Gdk::BUTTON_PRESS_MASK
                              | Gdk::BUTTON_RELEASE_MASK
                              | Gdk::BUTTON_MOTION_MASK;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.wclass = GDK_INPUT_OUTPUT;

        _gdk_window = Gdk::Window::create(
            get_parent_window(),
            &attributes,
            GDK_WA_X | GDK_WA_Y);
        set_window(_gdk_window);

        override_background_color(Gdk::RGBA("red"));
        override_color(Gdk::RGBA("blue"));

        _gdk_window->set_user_data(gobj());
    }
}

void TileEditor::on_unrealize()
{
    _gdk_window.reset();
    Widget::on_unrealize();
}

bool TileEditor::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (is_sensitive() && !_tile->default_visual.frames.empty()) {
        ImageData &image = _tile->default_visual.frames[0].image;
        cr->set_source(
            get_temporary_cairo_surface_for_tile_image_data(&image),
            0, 0);

        Cairo::RefPtr<Cairo::Pattern> patt = cr->get_source();
        Cairo::Matrix mat = patt->get_matrix();
        mat.scale((double)image.width/(double)editor_size,
                  (double)image.height/(double)editor_size);
        patt->set_matrix(mat);
    } else {
        Gdk::Cairo::set_source_rgba(cr, _border_colour);
    }

    cr->rectangle(0, 0, editor_size, editor_size);
    cr->fill();

    if (!is_sensitive()) {
        return true;
    }

    int rectx = 0;
    int recty = 0;
    for (unsigned int yc = 0; yc < subdivision_count; yc++)
    {
        for (unsigned int xc = 0; xc < subdivision_count; xc++)
        {
            Gdk::Cairo::set_source_rgba(
                cr,
                (get_stamp_value(xc, yc) ? _cell_colour_block
                                         : _cell_colour));
            cr->rectangle(rectx, recty, cell_size, cell_size);
            cr->fill();

            rectx += cell_size + border_size;
        }
        recty += cell_size + border_size;
        rectx = 0;
    }

    return true;
}

bool TileEditor::on_button_press_event(GdkEventButton *event)
{
    if (!is_sensitive()) {
        return true;
    }

    unsigned int x, y;
    if (!hit_cell(event->x, event->y, x, y)) {
        return true;
    }

    if (event->button == 1) {
        start_painting(!get_stamp_value(x, y));
        set_stamp_value(x, y, _paint_value);
        return false;
    }

    return true;
}

bool TileEditor::on_button_release_event(GdkEventButton *event)
{
    if (!is_sensitive()) {
        return true;
    }

    if (event->button == 1) {
        finish_painting();
        return false;
    }

    return true;
}

bool TileEditor::on_motion_notify_event(GdkEventMotion *event)
{
    if (!is_sensitive()) {
        return true;
    }

    if (!_painting) {
        return true;
    }

    unsigned int x, y;
    if (!hit_cell(event->x, event->y, x, y)) {
        return true;
    }

    set_stamp_value(x, y, _paint_value);
    return false;
}

void TileEditor::flush_cell_stamp()
{
}

void TileEditor::switch_tile(const SharedTile &tile)
{
    if (_tile) {
        flush_cell_stamp();
    }
    _tile = tile;
    if (_tile) {
        update_cell_stamp();
        set_sensitive(true);
    } else {
        set_sensitive(false);
    }
    queue_draw_area(0, 0, editor_size, editor_size);
}

void TileEditor::update_cell_stamp()
{
    if (!_tile) {
        return;
    }

    _stamp = _tile->stamp;
    queue_draw_area(0, 0, editor_size, editor_size);
}
