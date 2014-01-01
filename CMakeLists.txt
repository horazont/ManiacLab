cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror -std=c++11 -Wno-mismatched-tags -Wno-unused-parameter -Wno-literal-suffix -DPNG_SKIP_SETJMP_CHECK")
add_subdirectory(structstream EXCLUDE_FROM_ALL)
add_subdirectory(PyEngine)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror -std=c++11 -pedantic -msse -msse2 -msse3 -mmmx -Wno-mismatched-tags -Wno-unused-parameter -Wno-unused-private-field -Wno-literal-suffix -DPNG_SKIP_SETJMP_CHECK")

get_property(PYENGINE_DEPENDENCIES DIRECTORY PyEngine PROPERTY PYENGINE_DEPENDENCIES)
get_property(PYENGINE_LINK_TARGETS DIRECTORY PyEngine PROPERTY PYENGINE_LINK_TARGETS)
get_property(PYENGINE_DEFINITIONS DIRECTORY PyEngine PROPERTY PYENGINE_DEFINITIONS)
get_property(PYENGINE_INCLUDE_DIRS DIRECTORY PyEngine PROPERTY PYENGINE_INCLUDE_DIRS)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)


set(LIB_SOURCES
    "src/io/Common.cpp"
    "src/io/StructstreamIntf.cpp"
    "src/io/LevelData.cpp"
    "src/io/TilesetData.cpp"
    "src/io/Data.cpp"
    "src/logic/GameObject.cpp"
    "src/logic/Movements.cpp"
    "src/logic/Stamp.cpp"
    "src/logic/Physics.cpp"
    "src/logic/Level.cpp"
    "src/logic/TestObject.cpp"
    "src/logic/PythonInterface.cpp"
)

set(GAME_SOURCES
    "src/game/Mode.cpp"
    "src/game/MainMenu.cpp"
    "src/game/Application.cpp"
)

set(EDITOR_SOURCES
    "src/editor/GTKUtils.cpp"
    "src/editor/Operation.cpp"
    "src/editor/Editor.cpp"
    "src/editor/TilesetEditee.cpp"
    "src/editor/TileOperations.cpp"
    "src/editor/TileEditor.cpp"
    "src/editor/TilesetOperations.cpp"
    "src/editor/TilesetEditor.cpp"
    "src/editor/TilesetEditDetails.cpp"
    "src/editor/UniqueNameDialog.cpp"
    "src/editor/VFSFileChooserDialog.cpp"
    "src/editor/DuplicateTile.cpp"
    "src/editor/OpenImage.cpp"
    "src/editor/LevelCollectionEditor.cpp"
    "src/editor/LevelCollectionEditee.cpp"
    "src/editor/RootWindow.cpp"
)

include_directories(${PYENGINE_INCLUDE_DIRS})
include_directories(structstream)
include_directories("src")
add_definitions(${PYENGINE_DEFINITIONS})

add_library(ml ${LIB_SOURCES})
target_link_libraries(ml ${PYENGINE_LINK_TARGETS} structstream++)

add_executable(ml-game "src/game/maniaclab.cpp" ${GAME_SOURCES})
add_dependencies(ml-game ${PYENGINE_DEPENDENCIES} structstream++ ml)
target_link_libraries(ml-game ${PYENGINE_LINK_TARGETS} "pthread" structstream++ ml)

include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(ml-edit "src/editor/ml-edit.cpp" ${EDITOR_SOURCES})
add_dependencies(ml-edit structstream++ ml)
target_link_libraries(ml-edit
    "pthread"
    structstream++
    ${GTKMM_LIBRARIES}
    ${PYENGINE_LINK_TARGETS}
    ml)
