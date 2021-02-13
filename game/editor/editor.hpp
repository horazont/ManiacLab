#ifndef EDITOR_H
#define EDITOR_H

#include "mode.hpp"

#include <memory>

#include <QAbstractListModel>

#include <ffengine/common/types.hpp>
#include <ffengine/math/vector.hpp>

#include "logic/types.hpp"
#include "logic/tileset.hpp"
#include "logic/editor_level.hpp"

#include "drag.hpp"

namespace Ui {
class Editor;
}

class Level;
struct EditorScene;

static constexpr int ROLE_TILE_UUID = Qt::UserRole + 1;

class TilesetModel: public QAbstractListModel
{
public:
    TilesetModel(const Tileset &tileset);
    ~TilesetModel() override;

private:
    const TilesetTileContainer &m_tiles;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
};

class Editor : public ApplicationMode
{
    Q_OBJECT

public:
    enum MouseMode {
        MM_NONE = 0,
        MM_CAMERA_ZOOM = 2,
        MM_TOOL_DRAG = 3
    };

public:
    Editor(Application &app, QWidget *parent = nullptr);
    ~Editor() override;

private:
    std::unique_ptr<Ui::Editor> m_ui;
    std::unique_ptr<Tileset> m_tileset;
    std::unique_ptr<TilesetModel> m_model;

    QMetaObject::Connection m_advance_conn;
    QMetaObject::Connection m_after_gl_sync_conn;
    QMetaObject::Connection m_before_gl_sync_conn;

    QPoint m_local_mouse_pos;
    MouseMode m_mouse_mode;
    Vector2f m_camera_drag_handle_start;
    Vector2f m_camera_drag_pos_start;
    Qt::MouseButton m_drag_button;
    std::unique_ptr<AbstractToolDrag> m_active_mouse_drag;

    std::unique_ptr<EditorLevel> m_level;
    std::unique_ptr<EditorScene> m_scene;

    Vector2f m_viewport_size;

    float m_distort_t;

protected:
    Vector2f mouse_to_scene(const QPoint p);
    std::pair<bool, CoordPair> scene_to_world(const Vector2f p);

public slots:
    void advance(ffe::TimeInterval dt);
    void after_gl_sync();
    void before_gl_sync();

    void loadClicked(bool checked);
    void saveClicked(bool checked);


    // ApplicationMode interface
public:
    void activate(QWidget &parent) override;
    void deactivate() override;

    // QWidget interface
protected:
    void mouseMoveEvent(QMouseEvent *event) override;

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // QWidget interface
protected:
    void wheelEvent(QWheelEvent *event) override;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // EDITOR_H
