#include "Common.hpp"

/* LevelIOError */

LevelIOError::LevelIOError(const std::string &what):
    std::runtime_error(what)
{

}

LevelIOError::LevelIOError(const char *what):
    std::runtime_error(what)
{

}
