/**********************************************************************
File name: Tileset.hpp
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
#ifndef _ML_TILESET_H
#define _ML_TILESET_H

#include <string>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <CEngine/GL/AbstractImage.hpp>
#include <CEngine/IO/Stream.hpp>

#include "logic/Stamp.hpp"
#include "io/StructstreamIntf.hpp"

class Tileset;
class Tile;
class TileModel;

typedef boost::shared_ptr<Tileset> TilesetHandle;
typedef boost::shared_ptr<Tile> TileHandle;

class TileImage: PyEngine::GL::AbstractImage2D
{
public:
    TileImage();
    TileImage(const TileImage &ref);
    TileImage& operator= (const TileImage &ref);
    virtual ~TileImage();
private:
    int _width, _height;
    void *_pixels;
    double _duration;
public:
    static constexpr int BytesPerPixel = 4;
    inline const int size() const {
        return _width*_height*BytesPerPixel;
    };
    void clear();

    inline const double &get_duration() const { return _duration; };

    void set_duration(const double &value) { _duration = value; };

    TileImageRecordHandle get_image_record(SS::ID id) const;
    void set_image_record(TileImageRecordHandle rec);
public:
    virtual GLenum getFormat() const {
        return GL_RGBA;
    };
    virtual GLsizei getHeight() const {
        return _height;
    };
    virtual const void* getPixelData() const {
        return _pixels;
    };
    virtual GLsizei getPixelSize() const {
        return BytesPerPixel;
    };
    virtual GLenum getType() const {
        return GL_UNSIGNED_BYTE;
    };
    virtual GLsizei getWidth() const {
        return _width;
    };
    virtual void texImage2D(const GLenum target, const GLint level,
                            const GLint internal_format) const;
    virtual void texSubImage2D(const GLenum target, const GLint level,
                               const GLint internal_format,
                               const GLint x, const GLint y) const;
};

typedef SS::struct_decl<
    SS::Container,
    ML_TILE_IMAGE_CONT,
    SS::struct_members<
        SS::member_cb<SS::Float64Record, ML_TILE_IMAGE_DURATION,
                      TileImage, double,
                      &TileImage::get_duration, &TileImage::set_duration>,
        SS::member_direct<TileImageRecord, ML_TILE_IMAGE_IMAGE,
                          TileImage,
                          &TileImage::get_image_record, &TileImage::set_image_record>
        >
    > tile_image_decl;

class TileModel
{
public:
    TileModel();
    TileModel(const TileModel &ref);
    TileModel& operator= (const TileModel &ref);
    ~TileModel();
private:
    std::list<TileImage> images;
public:
    void add_image(TileImage &&image);
    typedef std::list<TileImage> ImageList;
    typedef typename ImageList::const_iterator ImageListConstIter;

    ImageListConstIter cbegin() const;
    ImageListConstIter cend() const;
};

typedef SS::struct_decl<
    SS::Container,
    ML_TILE_MODEL_CONT,
    SS::struct_members<
        SS::member_sequence_cb<
            TileModel,
            tile_image_decl,
            TileModel::ImageListConstIter,
            &TileModel::cbegin,
            &TileModel::cend,
            &TileModel::add_image>
        >
    > tile_model_decl;

class Tile
{
public:
    Tile();
    Tile(const Tile &ref);
    Tile& operator= (const Tile &ref);
    ~Tile();
public:
    /* generic metadata */
    std::string unique_name;
    std::string display_name;

    /* game physics info */
    bool is_gravity_affected;
    bool is_rollable;
    bool is_sticky;
    bool is_edible;
    double temp_coefficient;
    double roll_radius;
    BoolCellStamp ca_stamp;

    /* visual info */
    TileModel model;
};

// typedef SS::struct_decl<
//     SS::Container,
//     ML_TILE_MODEL_CONT,
//     SS::struct_members<>
//     > tile_model_decl;


typedef SS::struct_decl<
    SS::Container,
    ML_TILE_CONT,
    SS::struct_members<
        SS::member_string<
            SS::UTF8Record, ML_TILE_UNIQUE_NAME,
            Tile, &Tile::unique_name>,
        SS::member_string<
            SS::UTF8Record, ML_TILE_DISPLAY_NAME,
            Tile, &Tile::display_name>,
        SS::member<
            SS::BoolRecord, ML_TILE_IS_GRAVITY_AFFECTED,
            Tile, bool, &Tile::is_gravity_affected>,
        SS::member<
            SS::BoolRecord, ML_TILE_IS_ROLLABLE,
            Tile, bool, &Tile::is_rollable>,
        SS::member<
            SS::BoolRecord, ML_TILE_IS_STICKY,
            Tile, bool, &Tile::is_sticky>,
        SS::member<
            SS::BoolRecord, ML_TILE_IS_EDIBLE,
            Tile, bool, &Tile::is_edible>,
        SS::member<
            SS::Float64Record, ML_TILE_TEMP_COEFFICIENT,
            Tile, double, &Tile::temp_coefficient>,
        SS::member<
            SS::Float64Record, ML_TILE_ROLL_RADIUS,
            Tile, double, &Tile::roll_radius>
        // SS::member_struct<
        //     Tile,
        //     tile_model_decl,
        //     &Tile::model>
        >
    > tile_decl;

struct TilesetReference
{
    std::string referenced;
    std::unordered_set<std::string> excluded_tiles;
};

typedef SS::struct_decl<
    SS::Container,
    ML_TILESET_REFERENCE_CONT,
    SS::struct_members<
        SS::member_string<SS::UTF8Record, ML_TILESET_REFERENCE_NAME,
                          TilesetReference, &TilesetReference::referenced>,
        SS::member_struct<
            TilesetReference,
            SS::container<
                SS::value_decl<SS::UTF8Record, ML_TILESET_REFERENCE_EXCLUDE, std::string>,
                std::insert_iterator<decltype(TilesetReference::excluded_tiles)>
                >,
            &TilesetReference::excluded_tiles
            >
        >
    > tileset_reference_decl;

class Tileset
{
public:
    Tileset();
    virtual ~Tileset();
private:
    /* generic metadata */
    std::string _unique_name;
    std::string _display_name;
    std::string _author;
    std::string _description;

    /* referenced tilesets */
    std::list<TilesetReference> _referenced_tilesets;

    /* tiles */
    std::list<TileHandle> _tiles;
public:
    inline const std::string &get_unique_name() const { return _unique_name; };
    inline const std::string &get_display_name() const { return _display_name; };
    inline const std::string &get_author() const { return _author; };
    inline const std::string &get_description() const { return _description; };

    void set_unique_name(const std::string &value) { _unique_name = value; };
    void set_display_name(const std::string &value) { _display_name = value; };
    void set_author(const std::string &value) { _author = value; };
    void set_description(const std::string &value) { _description = value; };

    void add_referenced_tileset(TilesetReference tileset_reference);
    void add_referenced_tileset(TilesetReference &&tileset_reference);

    void add_tile(TileHandle tile);
    void add_tile(TileHandle &&tile);
public:
    typedef decltype(_referenced_tilesets) ReferencedTilesets;
    typedef typename ReferencedTilesets::const_iterator ReferencedTilesetsConstIter;

    inline ReferencedTilesetsConstIter referenced_tilesets_cbegin() const
    {
        return _referenced_tilesets.cbegin();
    }

    inline ReferencedTilesetsConstIter referenced_tilesets_cend() const
    {
        return _referenced_tilesets.cend();
    }

    typedef decltype(_tiles) Tiles;
    typedef typename Tiles::const_iterator TilesConstIter;

    inline TilesConstIter tiles_cbegin() const
    {
        return _tiles.cbegin();
    }

    inline TilesConstIter tiles_cend() const
    {
        return _tiles.cend();
    }
public:
    void save_to_stream(PyEngine::StreamHandle stream) const;
    static Tileset *load_from_stream(PyEngine::StreamHandle stream);
};

typedef SS::struct_decl<
    SS::Container,
    ML_TILESET_CONT,
    SS::struct_members<
        SS::member_string_cb<SS::UTF8Record, ML_TILESET_UNIQUE_NAME,
                             Tileset, &Tileset::get_unique_name, &Tileset::set_unique_name>,
        SS::member_string_cb<SS::UTF8Record, ML_TILESET_DISPLAY_NAME,
                             Tileset, &Tileset::get_display_name, &Tileset::set_display_name>,
        SS::member_string_cb<SS::UTF8Record, ML_TILESET_AUTHOR,
                             Tileset, &Tileset::get_author, &Tileset::set_author>,
        SS::member_string_cb<SS::UTF8Record, ML_TILESET_DESCRIPTION,
                             Tileset, &Tileset::get_description, &Tileset::set_description>,
        SS::member_sequence_cb<
            Tileset,
            tileset_reference_decl,
            Tileset::ReferencedTilesetsConstIter,
            &Tileset::referenced_tilesets_cbegin,
            &Tileset::referenced_tilesets_cend,
            &Tileset::add_referenced_tileset
            >,
        SS::member_sequence_cb<
            Tileset,
            tile_decl,
            Tileset::TilesConstIter,
            &Tileset::tiles_cbegin,
            &Tileset::tiles_cend,
            &Tileset::add_tile
            >
        >
    > tileset_decl;

struct TilesetInfo {
    std::string unique_name;
    std::string display_name;
    std::string author;
    std::string description;

    static TilesetInfo *read_from_stream(PyEngine::StreamHandle stream);
};

typedef boost::shared_ptr<TilesetInfo> TilesetInfoHandle;

typedef SS::struct_decl<
    SS::Container,
    ML_TILESET_CONT,
    SS::struct_members<
        SS::member_string<SS::UTF8Record, ML_TILESET_UNIQUE_NAME,
                          TilesetInfo, &TilesetInfo::unique_name>,
        SS::member_string<SS::UTF8Record, ML_TILESET_DISPLAY_NAME,
                          TilesetInfo, &TilesetInfo::display_name>,
        SS::member_string<SS::UTF8Record, ML_TILESET_AUTHOR,
                          TilesetInfo, &TilesetInfo::author>,
        SS::member_string<SS::UTF8Record, ML_TILESET_DESCRIPTION,
                          TilesetInfo, &TilesetInfo::description>
        >
    > tileset_info_decl;

namespace StructStream {

template <typename type>
struct iterator_helper<type, boost::shared_ptr<type>>
{
    static inline void assign(boost::shared_ptr<type> *src, type **dest)
    {
        *dest = src->get();
    }

    static inline boost::shared_ptr<type> construct()
    {
        return boost::shared_ptr<type>(new type());
    }
};

template <typename type>
struct iterator_helper<const type, const boost::shared_ptr<type>>
{
    static inline void assign(const boost::shared_ptr<type> *src, const type **dest)
    {
        *dest = src->get();
    }
};

}

#endif
