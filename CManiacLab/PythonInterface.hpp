#ifndef _ML_PYTHON_INTERFACE_H
#define _ML_PYTHON_INTERFACE_H

#include <python.h>

#include "GameCore.hpp"

class PyGameObject: public GameObject {
public:
    PyGameObject();
    virtual ~PyGameObject() {}; 
};

void addManiacLabToInittab();

#endif
