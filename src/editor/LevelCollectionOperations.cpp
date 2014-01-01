#include "LevelCollectionOperations.hpp"

/* LevelCollectionOperation */

LevelCollectionOperation::LevelCollectionOperation(
        LevelCollectionEditee *level_collection):
    Operation(),
    _level_collection(level_collection)
{

}

/* OpNewLevel */

OpNewLevel::OpNewLevel(
        LevelCollectionEditee *level_collection):
    LevelCollectionOperation(level_collection),
    _level()
{

}

void OpNewLevel::execute()
{
    if (_level) {
        _level = _level_collection->add_level(_level);
    } else {
        _level = _level_collection->new_level(
            PyEngine::UUID::with_generator<PyEngine::uuid_random>());
    }
}

void OpNewLevel::undo()
{
    _level_collection->delete_level(_level);
}

/* OpNewLevel */

OpDeleteLevel::OpDeleteLevel(
        LevelCollectionEditee *level_collection,
        const SharedLevel &level):
    LevelCollectionOperation(level_collection),
    _level(level)
{

}

void OpDeleteLevel::execute()
{
    _level_collection->delete_level(_level);
}

void OpDeleteLevel::undo()
{
    _level = _level_collection->add_level(_level);
}
