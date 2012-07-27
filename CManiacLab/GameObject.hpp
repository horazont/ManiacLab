#ifndef _ML_GAME_OBJECT_H
#define _ML_GAME_OBJECT_H

struct Cell;
class Level;

class GameObject {
public:
    GameObject();
    virtual ~GameObject() {};
protected:
    bool _isGravityAffected, _isMoving, _isRollable;
    double _tempCoefficient;
    double _x, _y, _phi, _radius;
public:
    bool inline getIsGravityAffected() const { return _isGravityAffected; };
    bool inline getIsMoving() const { return _isMoving; };
    bool inline getIsRollable() const { return _isRollable; };
    double inline getRadius() const { return _radius; };
    double inline getTemperatureCoefficient() const { return _tempCoefficient; };
friend class Level;
};

#endif
