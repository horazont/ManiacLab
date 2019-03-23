#ifndef EDITOR_H
#define EDITOR_H

#include "mode.hpp"

#include <memory>

#include <QAbstractListModel>

#include "logic/tileset.hpp"

namespace Ui {
class Editor;
}

class TilesetModel: public QAbstractListModel
{
public:
    TilesetModel(AbstractTileset &tileset);
    ~TilesetModel() override;

public:

};

class Editor : public ApplicationMode
{
    Q_OBJECT

public:
    Editor(Application &app, QWidget *parent = nullptr);
    ~Editor() override;

private:
    std::unique_ptr<Ui::Editor> m_ui;

};

#endif // EDITOR_H
