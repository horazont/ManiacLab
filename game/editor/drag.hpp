/**********************************************************************
File name: drag.hpp
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
#ifndef GAME_TERRAFORM_DRAG_HPP
#define GAME_TERRAFORM_DRAG_HPP

#include "fixups.hpp"

#include <functional>

#include <QCursor>

#include "ffengine/math/plane.hpp"
#include "ffengine/math/vector.hpp"

#include "ffengine/render/camera.hpp"
#include "ffengine/render/scenegraph.hpp"


class AbstractToolDrag
{
public:
    explicit AbstractToolDrag(bool continuous);
    virtual ~AbstractToolDrag();

private:
    bool m_continuous;
    bool m_use_cursor;
    QCursor m_cursor;

protected:
    void set_cursor(const QCursor &cursor);
    void reset_cursor();

public:
    inline const QCursor &cursor() const
    {
        return m_cursor;
    }

    inline bool is_continuous() const
    {
        return m_continuous;
    }

    inline bool use_cursor() const
    {
        return m_use_cursor;
    }

public:
    virtual void done(const Vector2f &viewport_pos) = 0;
    virtual void drag(const Vector2f &viewport_pos) = 0;

};


class WorldToolDrag: public AbstractToolDrag
{
public:
    using DragCallback = std::function<void(const Vector2f&, const Vector2f&)>;
    using DoneCallback = std::function<void(const Vector2f&, const Vector2f&)>;

public:
    explicit WorldToolDrag(const ffe::OrthogonalCamera &camera,
                           const Vector2f &viewport_size,
                           DragCallback &&drag_cb,
                           DoneCallback &&done_cb);

private:
    const ffe::OrthogonalCamera &m_camera;
    const Vector2f m_viewport_size;
    DragCallback m_drag_cb;
    DoneCallback m_done_cb;

private:
    Vector2f viewport_to_scene(const Vector2f vp);

    // AbstractToolDrag interface
public:
    void done(const Vector2f &viewport_pos) override;
    void drag(const Vector2f &viewport_pos) override;

};


class CustomToolDrag: public AbstractToolDrag
{
public:
    using DragCallback = std::function<void(const Vector2f&)>;
    using DoneCallback = std::function<void(const Vector2f&)>;

public:
    explicit CustomToolDrag(DragCallback &&drag_cb,
                            DoneCallback &&done_cb = nullptr,
                            bool continuous = false);

private:
    DragCallback m_drag_cb;
    DoneCallback m_done_cb;

public:
    void done(const Vector2f &viewport_pos);
    void drag(const Vector2f &viewport_pos);


};


#endif
