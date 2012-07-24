cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++0x")
add_subdirectory(PyEngine)

get_property(PYENGINE_DEPENDENCIES DIRECTORY PyEngine PROPERTY PYENGINE_DEPENDENCIES)
get_property(PYENGINE_LINK_TARGETS DIRECTORY PyEngine PROPERTY PYENGINE_LINK_TARGETS)
get_property(PYENGINE_DEFINITIONS DIRECTORY PyEngine PROPERTY PYENGINE_DEFINITIONS)
get_property(PYENGINE_INCLUDE_DIRS DIRECTORY PyEngine PROPERTY PYENGINE_INCLUDE_DIRS)

include_directories(${PYENGINE_INCLUDE_DIRS})
add_definitions(${PYENGINE_DEFINITIONS})

add_executable(maniaclab maniaclab.cpp)
add_dependencies(maniaclab ${PYENGINE_DEPENDENCIES})
target_link_libraries(maniaclab ${PYENGINE_LINK_TARGETS})
