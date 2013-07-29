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

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)


set(LIB_SOURCES
    "src/io/StructstreamIntf.cpp"
    "src/io/TilesetData.cpp"
    "src/logic/GameObject.cpp"
    "src/logic/Movements.cpp"
    "src/logic/Stamp.cpp"
    "src/logic/Physics.cpp"
    "src/logic/Level.cpp"
    "src/logic/TestObject.cpp"
    "src/logic/PythonInterface.cpp"
)

set(GAME_SOURCES
    ${LIB_SOURCES}
    "src/game/Mode.cpp"
    "src/game/MainMenu.cpp"
    "src/game/Application.cpp"
)

set(EDITOR_SOURCES
    ${LIB_SOURCES}
    "src/editor/RootWindow.cpp"
)

include_directories(${PYENGINE_INCLUDE_DIRS})
include_directories(structstream)
include_directories("src")
add_definitions(${PYENGINE_DEFINITIONS})

add_executable(ml "src/game/maniaclab.cpp" ${GAME_SOURCES})
add_dependencies(ml ${PYENGINE_DEPENDENCIES} structstream++)
target_link_libraries(ml ${PYENGINE_LINK_TARGETS} "pthread" structstream++)

include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(ml-edit "src/editor/ml-edit.cpp" ${EDITOR_SOURCES})
add_dependencies(ml-edit structstream++)
target_link_libraries(ml-edit
    "pthread"
    structstream++
    ${GTKMM_LIBRARIES}
    ${PYENGINE_LINK_TARGETS})
