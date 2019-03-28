#ifndef EDITOR_H
#define EDITOR_H

#include "mode.hpp"

#include <memory>

#include <QAbstractListModel>

#include <ffengine/common/types.hpp>

#include "logic/tileset.hpp"

namespace Ui {
class Editor;
}

class Level;
struct EditorScene;

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
    Editor(Application &app, QWidget *parent = nullptr);
    ~Editor() override;

private:
    std::unique_ptr<Ui::Editor> m_ui;
    std::unique_ptr<TilesetModel> m_model;

    QMetaObject::Connection m_advance_conn;
    QMetaObject::Connection m_after_gl_sync_conn;
    QMetaObject::Connection m_before_gl_sync_conn;

    QPoint m_local_mouse_pos;

    std::unique_ptr<Level> m_level;
    std::unique_ptr<EditorScene> m_scene;

    float m_distort_t;

public slots:
    void advance(ffe::TimeInterval dt);
    void after_gl_sync();
    void before_gl_sync();


    // ApplicationMode interface
public:
    void activate(QWidget &parent) override;
    void deactivate() override;

    // QWidget interface
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // EDITOR_H
