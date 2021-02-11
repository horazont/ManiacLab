#ifndef ML_EDITOR_TOOLS_H
#define ML_EDITOR_TOOLS_H

#include <QUuid>

#include <ffengine/math/vector.hpp>

#include "logic/types.hpp"
#include "logic/tileset.hpp"


class ToolBackend
{
public:
    ToolBackend();

public:
    Tileset *selected_tileset;
    QUuid selected_tile;

    Vector2f viewport_to_scene(const Vector2f &viewport);
    CoordPair scene_to_level(const Vector2f &scene);
    CoordPair scene_to_phy(const Vector2f &scene);

};


class AbstractTool
{
public:
    explicit AbstractTool(ToolBackend &backend);
    virtual ~AbstractTool();

public:
    virtual void primary_start(const Vector2f &viewport_pos);
    virtual void secondary_start(const Vector2f &viewport_pos);
    virtual void tertiary_start(const Vector2f &viewport_pos);

};

class PlacementTool: public AbstractTool
{
public:
    using AbstractTool::AbstractTool;

};

#endif
