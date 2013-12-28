/**********************************************************************
File name: TilesetOperations.hpp
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
#ifndef _ML_TILESET_OPERATIONS_H
#define _ML_TILESET_OPERATIONS_H

#include "Operation.hpp"

#include "TilesetEditee.hpp"

class TilesetOperation: public Operation
{
public:
    TilesetOperation(TilesetEditee *tileset);

protected:
    TilesetEditee *_tileset;

};

class OpNewTile: public TilesetOperation
{
public:
    OpNewTile(TilesetEditee *tileset);

private:
    SharedTile _tile;

public:
    void execute() override;
    void undo() override;

};

class OpDuplicateTile: public TilesetOperation
{
public:
    OpDuplicateTile(
        TilesetEditee *tileset,
        const SharedTile &src,
        bool rewrite_references_to_self);

private:
    const SharedTile _src;
    const bool _rewrite_references_to_self;
    SharedTile _tile;

public:
    void execute() override;
    void undo() override;

};

class OpDeleteTile: public TilesetOperation
{
public:
    OpDeleteTile(TilesetEditee *tileset,
                 const SharedTile &tile);

private:
    SharedTile _tile;

public:
    void execute() override;
    void undo() override;

};

template <typename value_t>
class OpSetTileAttribute: public TilesetOperation
{
public:
    OpSetTileAttribute(
            TilesetEditee *tileset,
            const SharedTile &tile,
            const value_t &value):
        TilesetOperation(tileset),
        _tile(tile),
        _new_value(value),
        _old_value()
    {

    };

protected:
    SharedTile _tile;
    const value_t _new_value;
    value_t _old_value;

protected:
    virtual const value_t &get_value() const = 0;
    virtual void set_value(const value_t &value) = 0;

public:
    void execute() override
    {
        _old_value = get_value();
        set_value(_new_value);
    };

    void undo() override
    {
        set_value(_old_value);
    };
};

template <typename value_t,
          value_t (TileData::*member_ptr),
          void (TilesetEditee::*setter_func)(const SharedTile&, value_t)>
class OpSetTileAttributePtr: public OpSetTileAttribute<value_t>
{
public:
    OpSetTileAttributePtr(
            TilesetEditee *tileset,
            const SharedTile &tile,
            const value_t &value):
        OpSetTileAttribute<value_t>(tileset, tile, value)
    {

    };

protected:
    virtual const value_t &get_value() const
    {
        return this->_tile.get()->*member_ptr;
    };

    virtual void set_value(const value_t &value)
    {
        (this->_tileset->*setter_func)(this->_tile, value);
    };
};

class OpSetTileDisplayName: public OpSetTileAttribute<std::string>
{
public:
    OpSetTileDisplayName(TilesetEditee *tileset,
                         const SharedTile &tile,
                         const std::string &value);

protected:
    const std::string &get_value() const override;
    void set_value(const std::string &value) override;

};

typedef OpSetTileAttributePtr<bool, &TileData::is_actor, &TilesetEditee::set_tile_actor> OpSetTileActor;
typedef OpSetTileAttributePtr<bool, &TileData::is_blocking, &TilesetEditee::set_tile_blocking> OpSetTileBlocking;
typedef OpSetTileAttributePtr<bool, &TileData::is_destructible, &TilesetEditee::set_tile_destructible> OpSetTileDestructible;
typedef OpSetTileAttributePtr<bool, &TileData::is_edible, &TilesetEditee::set_tile_edible> OpSetTileEdible;
typedef OpSetTileAttributePtr<bool, &TileData::is_gravity_affected, &TilesetEditee::set_tile_gravity_affected> OpSetTileGravityAffected;
typedef OpSetTileAttributePtr<bool, &TileData::is_movable, &TilesetEditee::set_tile_movable> OpSetTileMovable;
typedef OpSetTileAttributePtr<bool, &TileData::is_rollable, &TilesetEditee::set_tile_rollable> OpSetTileRollable;
typedef OpSetTileAttributePtr<bool, &TileData::is_sticky, &TilesetEditee::set_tile_sticky> OpSetTileSticky;

typedef OpSetTileAttributePtr<float, &TileData::roll_radius, &TilesetEditee::set_tile_roll_radius> OpSetTileRollRadius;
typedef OpSetTileAttributePtr<float, &TileData::temp_coefficient, &TilesetEditee::set_tile_temp_coefficient> OpSetTileTempCoefficient;

#endif
