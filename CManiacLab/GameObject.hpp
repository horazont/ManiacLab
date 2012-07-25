#ifndef _ML_GAME_OBJECT_H
#define _ML_GAME_OBJECT_H

struct Cell;

class GameObject {
public:
    GameObject();
    virtual ~GameObject() {};
protected:
    double _tempCoefficient;
public:
    double inline getTemperatureCoefficient() { return _tempCoefficient; };
    void update();
};

#endif
