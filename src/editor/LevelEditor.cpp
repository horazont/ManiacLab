#include "LevelEditor.hpp"

#include "RootWindow.hpp"

using namespace Gtk;

/* LevelEditor */

LevelEditor::LevelEditor(RootWindow *root, LevelCollectionEditee *editee):
    Glib::ObjectBase("leveleditor"),
    Gtk::Widget(),
    _root(root),
    _collection(editee),
    _level(nullptr),
    _background(Gdk::RGBA("black")),
    _border_colour(Gdk::RGBA("#666")),
    _border_colour_hover(Gdk::RGBA("#eee")),
    _hover(false)
{
    set_has_window(true);
    set_sensitive(false);
}

LevelEditor::~LevelEditor()
{

}

void LevelEditor::cell_to_client(unsigned int x, unsigned int y,
                                 unsigned int &coordx, unsigned int &coordy)
{
    const unsigned int total_cell_size = cell_size + border_width;
    const unsigned int half_border_width = border_width / 2;

    coordx = x * total_cell_size + half_border_width;
    coordy = y * total_cell_size + half_border_width;
}

bool LevelEditor::client_to_cell(int x, int y,
                                 unsigned int &cellx, unsigned int &celly)
{
    if ((x < 0) || (x >= (int)editor_width) ||
        (y < 0) || (y >= (int)editor_height))
    {
        return false;
    }

    const unsigned int total_cell_size = cell_size + border_width;

    cellx = x / total_cell_size;
    celly = y / total_cell_size;

    return true;

}

void LevelEditor::finish_painting()
{
    _painting = false;
    _paint_brush = std::make_pair(nullptr, nullptr);
    // FIXME: create & execute an operation
}

const SharedTile &LevelEditor::get_cell(unsigned int x, unsigned int y)
{
    return (*_level->get_tile_layer(TILELAYER_DEFAULT))[x + y * level_width].second;
}

bool LevelEditor::hit_cell(gdouble mousex, gdouble mousey,
                           unsigned int &x, unsigned int &y)
{
    int mx = round(mousex);
    int my = round(mousey);

    return client_to_cell(mx, my, x, y);
}

void LevelEditor::queue_draw_cell(unsigned int x, unsigned int y)
{
    unsigned int coordx, coordy;
    cell_to_client(x, y, coordx, coordy);

    queue_draw_area(coordx-border_width/2,
                    coordy-border_width/2,
                    cell_size+border_width,
                    cell_size+border_width);
}

void LevelEditor::set_cell(unsigned int x, unsigned int y,
                           const LevelData::TileBinding &brush)
{
    (*_level->get_tile_layer(TILELAYER_DEFAULT))[x + y * level_width] = brush;
    queue_draw_cell(x, y);
}

void LevelEditor::start_painting(const LevelData::TileBinding &brush)
{
    _painting = true;
    _paint_brush = brush;
}

void LevelEditor::get_preferred_height_for_width_vfunc(
    int width, int &minimum_height, int &natural_height) const
{
    return get_preferred_height_vfunc(minimum_height, natural_height);
}

void LevelEditor::get_preferred_height_vfunc(
    int &minimum_height, int &natural_height) const
{
    minimum_height = editor_height;
    natural_height = editor_height;
}

void LevelEditor::get_preferred_width_for_height_vfunc(
    int height, int &minimum_width, int &natural_width) const
{
    return get_preferred_width_vfunc(minimum_width, natural_width);
}

void LevelEditor::get_preferred_width_vfunc(
    int &minimum_width, int &natural_width) const
{
    minimum_width = editor_width;
    natural_width = editor_width;
}

SizeRequestMode LevelEditor::get_request_mode_vfunc() const
{
    return Widget::get_request_mode_vfunc();
}

void LevelEditor::on_size_allocate(Allocation &allocation)
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

void LevelEditor::on_map()
{
    Widget::on_map();
}

void LevelEditor::on_unmap()
{
    Widget::on_unmap();
}

void LevelEditor::on_realize()
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
                              | Gdk::BUTTON_MOTION_MASK
                              | Gdk::POINTER_MOTION_MASK;
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

void LevelEditor::on_unrealize()
{
    _gdk_window.reset();
    Widget::on_unrealize();
}

bool LevelEditor::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (!is_sensitive()) {
        Gdk::Cairo::set_source_rgba(cr, _border_colour);
        cr->rectangle(0, 0, editor_width, editor_height);
        cr->fill();
        return true;
    }

    Gdk::Cairo::set_source_rgba(cr, _border_colour);
    cr->rectangle(0, 0, editor_width, editor_height);
    cr->fill();

    for (unsigned x = 0; x < width; x++) {
        for (unsigned y = 0; y < height; y++) {
            unsigned int cellx, celly;
            cell_to_client(x, y, cellx, celly);

            Gdk::Cairo::set_source_rgba(cr, _background);
            cr->rectangle(cellx, celly, cell_size, cell_size);
            cr->fill_preserve();

            const SharedTile &tile = get_cell(x, y);
            if (tile) {
                Cairo::RefPtr<Cairo::ImageSurface> pic = _root->get_tile_picture(
                    tile);

                if (pic) {
                    double scalex = (double)pic->get_width()/(double)cell_size;
                    double scaley = (double)pic->get_height()/(double)cell_size;
                    cr->set_source(pic, cellx*scalex, celly*scaley);
                    Cairo::RefPtr<Cairo::Pattern> patt = cr->get_source();
                    Cairo::Matrix mat = patt->get_matrix();
                    mat.scale(scalex, scaley);
                    patt->set_matrix(mat);
                } else {
                    cr->set_source_rgba(1, 0, 1, 1);
                }
                cr->fill();
            } else {
                cr->begin_new_path();
            }

            if (_hover && _hover_cell == std::make_pair(x, y)) {
                cr->rectangle(cellx,
                              celly,
                              cell_size,
                              cell_size);
                Gdk::Cairo::set_source_rgba(cr, _border_colour_hover);
                cr->stroke();
            }
        }
    }

    return true;
}

bool LevelEditor::on_button_press_event(GdkEventButton *event)
{
    if (!is_sensitive()) {
        return true;
    }

    if (_painting) {
        return true;
    }

    unsigned int x, y;
    if (!hit_cell(event->x, event->y, x, y)) {
        return true;
    }

    LevelData::TileBinding *brush = nullptr;
    if (event->button == 1) {
        brush = &_primary_brush;
    } else if (event->button == 3) {
        brush = &_secondary_brush;
    } else {
        return true;
    }

    start_painting(*brush);
    set_cell(x, y, *brush);

    return true;
}

bool LevelEditor::on_button_release_event(GdkEventButton *event)
{
    if (!is_sensitive()) {
        return true;
    }

    if (event->button == 1 || event->button == 3) {
        finish_painting();
        return true;
    }

    return true;
}

bool LevelEditor::on_motion_notify_event(GdkEventMotion *event)
{
    if (!is_sensitive()) {
        return true;
    }

    unsigned int x, y;
    if (!hit_cell(event->x, event->y, x, y)) {
        if (_hover) {
            queue_draw_cell(_hover_cell.first,
                            _hover_cell.second);
        }
        _hover = false;
        return true;
    }

    if (_hover && _hover_cell != std::make_pair(x, y)) {
        queue_draw_cell(_hover_cell.first,
                        _hover_cell.second);
    }

    _hover = true;
    _hover_cell = std::make_pair(x, y);

    if (_painting) {
        set_cell(x, y, _paint_brush);
        return true;
    } else {
        queue_draw_cell(x, y);
    }

    return true;
}

void LevelEditor::set_secondary_brush(const LevelData::TileBinding &brush)
{
    _secondary_brush = brush;
}

void LevelEditor::set_primary_brush(const LevelData::TileBinding &brush)
{
    _primary_brush = brush;
}

void LevelEditor::switch_level(const SharedLevel &level)
{
    _level = level;
    if (_level) {
        set_sensitive(true);
    } else {
        set_sensitive(false);
    }

    queue_draw_area(0, 0, editor_width, editor_height);
}
