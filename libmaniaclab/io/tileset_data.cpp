#include "tileset_data.hpp"

/* ImageData */

/* TileData */

TileData::TileData():
    uuid(),
    display_name(),
    is_actor(false),
    is_blocking(false),
    is_destructible(false),
    is_collectable(false),
    is_gravity_affected(false),
    is_movable(false),
    is_round(false),
    is_sticky(false),
    roll_radius(1),
    temp_coefficient(1),
    default_visual(),
    additional_visuals()
{

}

/* free functions */

/*std::pair<StreamSink, std::unique_ptr<TilesetData>>
    create_tileset_sink()
{
    std::unique_ptr<TilesetData> result(new TilesetData());
    StreamSink sink(new SinkChain({
        deserialize<only<TilesetHeaderDecl, true, true>>(result->header),
        deserialize<only<TilesetBodyDecl, true, true>>(result->body)
    }));

    return std::make_pair(sink, std::move(result));
}

std::unique_ptr<TilesetData> load_tileset_from_tree(const ContainerHandle &root)
{
    tree_debug(root, std::cout);

    std::unique_ptr<TilesetData> result;
    StreamSink sink;
    std::tie(sink, result) = create_tileset_sink();

    try {
        FromTree(sink, root);
    } catch (const RecordNotFound &err) {
        PyEngine::log->log(Error)
            << "Given tree is not a tileset: " << err.what() << submit;
        return nullptr;
    }

    return result;
}

std::unique_ptr<TilesetData> load_tileset_from_stream(const StreamHandle &stream)
{
    IOIntfHandle io(new PyEngineStream(stream));

    std::unique_ptr<TilesetData> result;
    StreamSink sink;
    std::tie(sink, result) = create_tileset_sink();

    try {
        FromBitstream(
            io,
            maniac_lab_registry,
            sink).read_all();
    } catch (const RecordNotFound &err) {
        PyEngine::log->log(Error)
            << "Given stream is not a tileset: " << err.what() << submit;
        return nullptr;
    } catch (const std::runtime_error &err) {
        PyEngine::log->log(Error)
            << "While loading tileset: " << err.what() << submit;
        return nullptr;
    }

    return result;
}

std::unique_ptr<TilesetHeaderData> load_tileset_header_from_stream(const StreamHandle &stream)
{
    std::unique_ptr<TilesetHeaderData> result(new TilesetHeaderData());
    IOIntfHandle io(new PyEngineStream(stream));

    try {
        FromBitstream(
            io,
            maniac_lab_registry,
            deserialize<only<TilesetHeaderDecl, true, true>>(*result.get())).read_all();
    } catch (const RecordNotFound &err) {
        return nullptr;
    } catch (const std::runtime_error &err) {
        PyEngine::log->log(Error)
            << "While loading tileset: " << err.what() << submit;
        return nullptr;
    }

    return result;
}*/
