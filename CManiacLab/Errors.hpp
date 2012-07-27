#ifndef _ML_ERRORS_H
#define _ML_ERRORS_H

#include "CEngine/Misc/Exception.hpp"

class ProgrammingError: public PyEngine::Exception {
public:
    ProgrammingError(const std::string &message): PyEngine::Exception(message) {};
    ProgrammingError(const char *message): PyEngine::Exception(message) {};
};

#endif
