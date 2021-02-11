#ifndef ML_PLAYER_OBJECT_H
#define ML_PLAYER_OBJECT_H

#include "logic/game_object.hpp"

struct PlayerController
{
    PlayerController();
    PlayerController(const PlayerController &ref) = delete;
    PlayerController &operator=(const PlayerController &ref) = default;
    PlayerController(PlayerController &&src) = delete;
    PlayerController &operator=(PlayerController &&src) = default;

    enum ActionRequest {
        AR_NONE = 0,
        AR_MOVE_UP,
        AR_MOVE_DOWN,
        AR_MOVE_LEFT,
        AR_MOVE_RIGHT,
        AR_COLLECT_UP,
        AR_COLLECT_DOWN,
        AR_COLLECT_LEFT,
        AR_COLLECT_RIGHT,
    };

    ActionRequest action_request;
};

class PlayerObject: public GameObject
{
public:
    explicit PlayerObject(Level &level);

private:
    PlayerController m_controller;

    bool try_collect(LevelCell &target);

    // GameObject interface
public:
    bool idle() override;

    inline PlayerController &controller()
    {
        return m_controller;
    }
};


#endif
