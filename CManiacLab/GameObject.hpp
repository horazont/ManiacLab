#ifndef _ML_GAME_OBJECT_H
#define _ML_GAME_OBJECT_H

#include "Movements.hpp"

struct Cell;
class Level;
class Movement;

class GameObject {
public:
    GameObject();
    virtual ~GameObject() {};
protected:
    bool _isGravityAffected, _isMoving, _isRollable;
    double _tempCoefficient;
    double _x, _y, _phi, _radius;
    Movement *_movement;
protected:
    void setMovement(Movement *movement);
public:
    Movement *getCurrentMovement() const { return _movement; };
    bool inline getIsGravityAffected() const { return _isGravityAffected; };
    bool inline getIsMoving() const { return _isMoving; };
    bool inline getIsRollable() const { return _isRollable; };
    double inline getRadius() const { return _radius; };
    double inline getTemperatureCoefficient() const { return _tempCoefficient; };
friend class MovementStraight;
friend class MovementRoll;
friend class Movement;
friend class Level;
};

#endif
