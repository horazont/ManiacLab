#ifndef _ML_GAME_OBJECT_H
#define _ML_GAME_OBJECT_H

struct Cell;

class GameObject {
public:
    virtual ~GameObject() {};
protected:
    double _tempCoefficient;
public:
    double inline getTemperatureCoefficient() { return _tempCoefficient; };
    virtual void update(const Cell *cell);
};

#endif
