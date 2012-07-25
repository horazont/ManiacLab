#ifndef _ML_GAME_OBJECT_H
#define _ML_GAME_OBJECT_H

struct Cell;

class GameObject {
public:
    GameObject();
    virtual ~GameObject() {};
protected:
    bool _isGravityAffected, _isMoving, _isRollable;
    double _tempCoefficient;
public:
    bool inline getIsGravityAffected() const { return _isGravityAffected; };
    bool inline getIsMoving() const { return _isMoving; };
    bool inline getIsRollable() const { return _isRollable; };
    double inline getTemperatureCoefficient() const { return _tempCoefficient; };
};

#endif
