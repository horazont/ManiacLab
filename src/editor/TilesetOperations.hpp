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
    OpNewTile(TilesetEditee *tileset,
              const std::string &unique_name);

private:
    const std::string _unique_name;
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
        const std::string &unique_name,
        bool rewrite_references_to_self);

private:
    const SharedTile _src;
    const std::string _unique_name;
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

class OpSetTileActor: public OpSetTileAttribute<bool>
{
public:
    OpSetTileActor(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileBlocking: public OpSetTileAttribute<bool>
{
public:
    OpSetTileBlocking(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileDestructible: public OpSetTileAttribute<bool>
{
public:
    OpSetTileDestructible(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileEdible: public OpSetTileAttribute<bool>
{
public:
    OpSetTileEdible(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileGravityAffected: public OpSetTileAttribute<bool>
{
public:
    OpSetTileGravityAffected(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileMovable: public OpSetTileAttribute<bool>
{
public:
    OpSetTileMovable(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileRollable: public OpSetTileAttribute<bool>
{
public:
    OpSetTileRollable(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileSticky: public OpSetTileAttribute<bool>
{
public:
    OpSetTileSticky(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const bool &value);

protected:
    const bool &get_value() const override;
    void set_value(const bool &value) override;

};

class OpSetTileRollRadius: public OpSetTileAttribute<float>
{
public:
    OpSetTileRollRadius(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const float &value);

protected:
    const float &get_value() const override;
    void set_value(const float &value) override;

};

class OpSetTileTempCoefficient: public OpSetTileAttribute<float>
{
public:
    OpSetTileTempCoefficient(
        TilesetEditee *tileset,
        const SharedTile &tile,
        const float &value);

protected:
    const float &get_value() const override;
    void set_value(const float &value) override;

};


#endif
