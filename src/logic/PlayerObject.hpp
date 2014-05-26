#ifndef _ML_PLAYER_OBJECT_H
#define _ML_PLAYER_OBJECT_H

#include "GameObject.hpp"

struct PlayerView: public ObjectView
{
public:
    virtual void update(PyEngine::TimeFloat deltaT);

};

struct PlayerObject: public GameObject
{
public:
    PlayerObject(const ObjectInfo &info,
                 Level *level);

public:
    Acting acting;

public:
    bool idle() override;

};

#endif
