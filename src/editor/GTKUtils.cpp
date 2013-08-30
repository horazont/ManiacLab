/**********************************************************************
File name: GTKUtils.cpp
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
#include "GTKUtils.hpp"

#include <CEngine/IO/Log.hpp>

#include "io/StructstreamIntf.hpp"

using namespace Glib;
using namespace Gtk;

int message_dlg(
        Window &parent,
        const std::string &primary_text,
        const std::string &secondary_text,
        MessageType message_type,
        ButtonsType buttons)
{
    MessageDialog dlg(parent, primary_text, false, message_type,
                      buttons, true);
    dlg.set_secondary_text(secondary_text);
    return dlg.run();
}

sigc::connection bind_action(
    const RefPtr<Builder> &builder,
    const std::string &name,
    const Action::SlotActivate &slot,
    RefPtr<Action> *action_dest)
{
    RefPtr<Action> action;
    action = action.cast_dynamic(builder->get_object(name));
    if (action_dest) {
        *action_dest = action;
    }
    return action->signal_activate().connect(slot);
}

RefPtr<Gdk::Pixbuf> tile_image_data_to_pixbuf(ImageData *data)
{
    assert(data->format == TVF_BGRA);
    RefPtr<Gdk::Pixbuf> result = Gdk::Pixbuf::create(
        Gdk::COLORSPACE_RGB, true, 8, data->width, data->height);

    assert(result->get_rowstride() == 0);

    guint8 *buffer = result->get_pixels();
    memcpy(
        buffer,
        data->data.c_str(),
        data->width * data->height * get_pixel_size(data->format)
        );

    return result;
}

void luminance_to_bgra(const uint8_t *src, uint8_t *dst,
                       uint16_t w, uint16_t h,
                       intptr_t rowstride)
{
    const intptr_t base_stride = rowstride - w;

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            uint8_t grey = *src++;
            *dst++ = grey;
            *dst++ = grey;
            *dst++ = grey;
            *dst++ = 255; // alpha
        }
        src += base_stride;
    }
}

void luminance_alpha_to_bgra(const uint8_t *src, uint8_t *dst,
                             uint16_t w, uint16_t h,
                             intptr_t rowstride)
{
    const intptr_t base_stride = rowstride - w*2;

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            uint8_t grey = *src++;
            *dst++ = grey;
            *dst++ = grey;
            *dst++ = grey;
            *dst++ = *src++; // alpha
        }
        src += base_stride;
    }
}

void rgb_to_bgra(const uint8_t *src, uint8_t *dst,
                 uint16_t w, uint16_t h,
                 intptr_t rowstride)
{
    const intptr_t base_stride = rowstride - w*3;

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;
            *dst++ = b;
            *dst++ = g;
            *dst++ = r;
            *dst++ = 255; // alpha
        }
        src += base_stride;
    }
}

void rgba_to_bgra(const uint8_t *src, uint8_t *dst,
                  uint16_t w, uint16_t h,
                  intptr_t rowstride)
{
    const intptr_t base_stride = rowstride - w*4;

    for (unsigned int y = 0; y < h; y++) {
        for (unsigned int x = 0; x < w; x++) {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;
            *dst++ = b;
            *dst++ = g;
            *dst++ = r;
            *dst++ = *src++; // alpha
        }
        src += base_stride;
    }
}

bool pixbuf_to_tile_image_data(
    const RefPtr<Gdk::Pixbuf> &src, ImageData *data)
{
    unsigned int n_channels = src->get_n_channels();
    const intptr_t rowstride = src->get_rowstride();
    const bool has_alpha = src->get_has_alpha();

    if ((src->get_width() > std::numeric_limits<uint16_t>::max())
        || (src->get_height() > std::numeric_limits<uint16_t>::max()))
    {
        PyEngine::log->getChannel("GTKUtils")->log(PyEngine::Error)
            << "Image too large" << PyEngine::submit;
        return false;
    }

    const uint16_t width = src->get_width();
    const uint16_t height = src->get_height();

    if ((src->get_colorspace() != Gdk::COLORSPACE_RGB) ||
        (src->get_bits_per_sample() != 8))
    {
        PyEngine::log->getChannel("GTKUtils")->log(PyEngine::Error)
            << "Image has wrong format: "
            << "bits_per_sample = " << src->get_bits_per_sample() << "; "
            << "colorspace = " << src->get_colorspace() << "; "
            << PyEngine::submit;
        return false;
    }

    if (!(((n_channels == 2) && (has_alpha))
          || ((n_channels == 1) && (!has_alpha))
          || ((n_channels == 3) && (!has_alpha))
          || ((n_channels == 4) && (has_alpha))))
    {
        PyEngine::log->getChannel("GTKUtils")->log(PyEngine::Error)
            << "Image channel configuration unreadable: "
            << "n_channels = " << src->get_bits_per_sample() << "; "
            << "has_alpha = " << src->get_colorspace() << "; "
            << PyEngine::submit;
        return false;
    }

    const guint8 *src_buf = src->get_pixels();
    data->width = width;
    data->height = height;
    data->format = TVF_BGRA;

    data->data.reserve(width * height * get_pixel_size(data->format));

    uint8_t *dst_buf = &data->data.front();

    switch (n_channels) {
    case 1:
    {
        luminance_to_bgra(
            src_buf, dst_buf, width, height, rowstride);
        break;
    }
    case 2:
    {
        luminance_alpha_to_bgra(
            src_buf, dst_buf, width, height, rowstride);
        break;
    }
    case 3:
    {
        rgb_to_bgra(
            src_buf, dst_buf, width, height, rowstride);
        break;
    }
    case 4:
    {
        rgba_to_bgra(
            src_buf, dst_buf, width, height, rowstride);
        break;
    }
    default:
        // must not happen, we checked above
        assert(false);
        return false;
    }

    return true;
}

Cairo::RefPtr<Cairo::Surface>
    get_temporary_cairo_surface_for_tile_image_data(ImageData *data)
{
    Cairo::RefPtr<Cairo::Surface> result =
        Cairo::ImageSurface::create(
            &data->data.front(),
            Cairo::FORMAT_ARGB32,
            data->width,
            data->height,
            data->width*4);
    return result;
}
