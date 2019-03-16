project(ManiacLab)
cmake_minimum_required(VERSION 3.11)

add_subdirectory(ffengine)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ffengine/CMakeModules")

add_subdirectory(libmaniaclab)
add_subdirectory(game)
add_subdirectory(tests)
