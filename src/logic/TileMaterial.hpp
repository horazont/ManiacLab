#ifndef _ML_TILE_MATERIAL_H
#define _ML_TILE_MATERIAL_H

#include <unordered_map>

#include <CEngine/IO/Time.hpp>
#include <CEngine/GL/IndexBuffer.hpp>
#include <CEngine/GL/GeometryBuffer.hpp>

class Metatexture;

struct MetatextureObject
{
public:
    float cx, cy;
    PyEngine::TimeFloat t;
    PyEngine::GL::VertexIndexListHandle vertices;
    PyEngine::GL::StreamIndexBufferHandle index_buffer;

    void stream();

};



class Metatexture
{
public:
    Metatexture(
        const PyEngine::GL::GeometryBufferHandle &geometry_buffer);
    Metatexture(const Metatexture &ref) = delete;
    Metatexture &operator=(const Metatexture &ref) = delete;
    virtual ~Metatexture() = default;

public:
    const PyEngine::GL::GeometryBufferHandle geometry_buffer;

public:
    /**
     * Allocate a new tile in the geometry buffer associated with this
     * metatexture.
     *
     * @param cx center x coordinate of the tile
     * @param cy center y coordinate of the tile
     * @param t the age of the tile
     * @return A geometry buffer allocation representing the verticies
     * of the tile. The buffer slice has been filled with the correct
     * values already.
     */
    virtual std::unique_ptr<MetatextureObject> create_tile(
        const float cx, const float cy,
        const PyEngine::TimeFloat t) = 0;

    /**
     * Return the height of the metatexture in tile units.
     */
    virtual float get_height() const = 0;

    /**
     * Return the width of the metatexture in tile units.
     */
    virtual float get_width() const = 0;

    /**
     * Update an existing vertex allocation for this metatexture,
     * using the new values for cx, cy and t.
     */
    virtual void update_tile(
        MetatextureObject &tile,
        const float cx, const float cy,
        const PyEngine::TimeFloat t) = 0;

};

class SimpleMetatexture: public Metatexture
{
public:
    SimpleMetatexture(
        const PyEngine::GL::GeometryBufferHandle &geometry_buffer,
        const PyEngine::GL::StreamIndexBufferHandle &index_buffer,
        const GLuint textureid,
        const float x0,
        const float y0,
        const float x1,
        const float y1,
        const float width,
        const float height);

public:
    const PyEngine::GL::StreamIndexBufferHandle &index_buffer;
    const GLuint textureid;
    const float x0, y0, x1, y1;
    const float width, height;

private:
    void set_buffer(MetatextureObject &obj,
                    const float cx, const float cy);

public:
    std::unique_ptr<MetatextureObject> create_tile(
        const float cx, const float cy,
        const PyEngine::TimeFloat t) override;
    float get_height() const override;
    float get_width() const override;
    void update_tile(
        MetatextureObject &tile,
        const float cx, const float cy,
        const PyEngine::TimeFloat t) override;

};

class TileMaterial
{
public:
    const Metatexture *diffuse;
    const Metatexture *emmission;

};

class TileMaterialManager
{
public:
    TileMaterialManager();

private:
    std::unordered_map<std::string,
                       std::unique_ptr<Metatexture>> _metatextures;
    std::unordered_map<std::string,
                       std::unique_ptr<TileMaterial>> _materials;

public:
    TileMaterial *get_material(const std::string &key) const;
    Metatexture *get_metatexture(const std::string &key) const;

    template <typename... arg_ts>
    TileMaterial *new_material(
        const std::string &key,
        arg_ts... args)
    {
        TileMaterial *mat = new TileMaterial(args...);
        register_material(
            key,
            std::unique_ptr<TileMaterial>(mat));
        return mat;
    }

    void register_material(
        const std::string &key,
        std::unique_ptr<TileMaterial> &&mat);
    void register_metatexture(
        const std::string &key,
        std::unique_ptr<Metatexture> &&tex);

};

#endif
