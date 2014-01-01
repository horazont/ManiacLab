#ifndef _ML_LEVEL_COLLECTION_OPERATIONS_H
#define _ML_LEVEL_COLLECTION_OPERATIONS_H

#include "Operation.hpp"

#include "LevelCollectionEditee.hpp"

class LevelCollectionOperation: public Operation
{
public:
    LevelCollectionOperation(
        LevelCollectionEditee *level_collection);

protected:
    LevelCollectionEditee *_level_collection;

};

class OpNewLevel: public LevelCollectionOperation
{
public:
    OpNewLevel(LevelCollectionEditee *level_collection);

private:
    SharedLevel _level;

public:
    void execute() override;
    void undo() override;

};

class OpDeleteLevel: public LevelCollectionOperation
{
public:
    OpDeleteLevel(
        LevelCollectionEditee *level_collection,
        const SharedLevel &level);

private:
    SharedLevel _level;

public:
    void execute() override;
    void undo() override;

};

#endif
