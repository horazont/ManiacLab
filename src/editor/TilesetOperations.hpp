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
