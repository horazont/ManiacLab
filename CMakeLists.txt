cmake_minimum_required(VERSION 2.6)
#~ set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++11")
add_subdirectory(structstream EXCLUDE_FROM_ALL)
add_subdirectory(PyEngine)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -std=c++11 -pedantic -msse -msse2 -msse3 -mmmx -O2")

get_property(PYENGINE_DEPENDENCIES DIRECTORY PyEngine PROPERTY PYENGINE_DEPENDENCIES)
get_property(PYENGINE_LINK_TARGETS DIRECTORY PyEngine PROPERTY PYENGINE_LINK_TARGETS)
get_property(PYENGINE_DEFINITIONS DIRECTORY PyEngine PROPERTY PYENGINE_DEFINITIONS)
get_property(PYENGINE_INCLUDE_DIRS DIRECTORY PyEngine PROPERTY PYENGINE_INCLUDE_DIRS)

set(MANIACLAB_SOURCES
    "CManiacLab/IOUtils.cpp"
    "CManiacLab/GameObject.cpp"
    "CManiacLab/Movements.cpp"
    "CManiacLab/Stamp.cpp"
    "CManiacLab/Physics.cpp"
    "CManiacLab/Level.cpp"
    "CManiacLab/TestObject.cpp"
    "CManiacLab/PythonInterface.cpp"
    "CManiacLab/StructstreamIntf.cpp"
    "CManiacLab/Tileset.cpp"
)

include_directories(${PYENGINE_INCLUDE_DIRS})
include_directories(structstream)
add_definitions(${PYENGINE_DEFINITIONS})

add_executable(ml maniaclab.cpp ${MANIACLAB_SOURCES})
add_dependencies(ml ${PYENGINE_DEPENDENCIES} structstream++)
target_link_libraries(ml ${PYENGINE_LINK_TARGETS} "pthread" structstream++)
