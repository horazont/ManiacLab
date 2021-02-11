/**********************************************************************
File name: drag.cpp
This file is part of: SCC (working title)

LICENSE

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about SCC please e-mail one of the authors named in
the AUTHORS file.
**********************************************************************/
#include "drag.hpp"

#include "ffengine/math/intersect.hpp"


/* AbstractToolDrag */

AbstractToolDrag::AbstractToolDrag(bool continuous):
    m_continuous(continuous),
    m_use_cursor(false)
{

}

AbstractToolDrag::~AbstractToolDrag()
{

}

void AbstractToolDrag::set_cursor(const QCursor &cursor)
{
    m_use_cursor = true;
    m_cursor = cursor;
}

void AbstractToolDrag::reset_cursor()
{
    m_use_cursor = false;
}

WorldToolDrag::WorldToolDrag(const ffe::OrthogonalCamera &camera,
                             const Vector2f &viewport_size,
                             DragCallback &&drag_cb,
                             DoneCallback &&done_cb):
    AbstractToolDrag(true),
    m_camera(camera),
    m_viewport_size(viewport_size),
    m_drag_cb(std::move(drag_cb)),
    m_done_cb(std::move(done_cb))
{

}

Vector2f WorldToolDrag::viewport_to_scene(const Vector2f vp)
{
    const Matrix4f inv_proj = std::get<1>(m_camera.render_projection(
                                              m_viewport_size[eX],
                                              m_viewport_size[eY]
                                              ));
    const Matrix4f inv_view = m_camera.calc_inv_view();
    const Vector4f ndc(2.f * vp[eX] / m_viewport_size[eX] - 1.f,
                       -(2.f * vp[eY] / m_viewport_size[eY] - 1.f),
                       0.f,
                       1.f);
    const Vector4f scene_pos = inv_view * inv_proj * ndc;
    return Vector2f(scene_pos[eX], scene_pos[eY]);
}

void WorldToolDrag::done(const Vector2f &viewport_pos)
{
    if (!m_done_cb) {
        return;
    }

    const Vector2f scene_pos = viewport_to_scene(viewport_pos);
    m_done_cb(viewport_pos, scene_pos);
}

void WorldToolDrag::drag(const Vector2f &viewport_pos)
{
    if (!m_drag_cb) {
        return;
    }

    const Vector2f scene_pos = viewport_to_scene(viewport_pos);
    m_drag_cb(viewport_pos, scene_pos);
}
