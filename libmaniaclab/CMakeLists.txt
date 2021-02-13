cmake_minimum_required(VERSION 3.11)

find_package(OpenGL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(SIGC++ REQUIRED)

set(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/")

set(MANIACLIB_HEADERS
    logic/level.hpp
    logic/movements.hpp
    logic/particles.hpp
    logic/physics.hpp
    logic/physicsconfig.hpp
    logic/sensor.hpp
    logic/stamp.hpp
    logic/tileset.hpp
    logic/types.hpp

    logic/bomb_object.hpp
    logic/builtin_tileset.hpp
    logic/dirt_object.hpp
    logic/explosion_object.hpp
    logic/fan_object.hpp
    logic/fog_object.hpp
    logic/game_object.hpp
    logic/player_object.hpp
    logic/rock_object.hpp
    logic/wall_object.hpp

    logic/server.hpp
    logic/editor_level.hpp

    render/visual_object.hpp
  )

set(MANIACLIB_SRC
    logic/level.cpp
    logic/movements.cpp
    logic/particles.cpp
    logic/physics.cpp
    logic/sensor.cpp
    logic/stamp.cpp
    logic/tileset.cpp

    logic/bomb_object.cpp
    logic/builtin_tileset.cpp
    logic/dirt_object.cpp
    logic/explosion_object.cpp
    logic/fan_object.cpp
    logic/fog_object.cpp
    logic/game_object.cpp
    logic/player_object.cpp
    logic/rock_object.cpp
    logic/wall_object.cpp

    logic/server.cpp
    logic/editor_level.cpp

    render/visual_object.cpp
    )

set(MANIACLIB_PROTOS
    proto/maniaclab.proto
    )

protobuf_generate_cpp(
  PROTO_SRCS
  PROTO_HDRS
  ${MANIACLIB_PROTOS})

add_library(maniaclib STATIC ${MANIACLIB_SRC} ${MANIACLIB_HEADERS} ${MANIACLIB_PROTOS} ${PROTO_SRCS} ${PROTO_HDRS})
setup_scc_target(maniaclib)

target_link_libraries(
  maniaclib
  ffengine-core
  ffengine-render
  sigc++
  ${OPENGL_LIBRARIES}
  ${OPENGL_glu_LIBRARY}
  ${Protobuf_LIBRARIES})
target_link_libraries(maniaclib "atomic")
target_include_directories(
  maniaclib
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(
  maniaclib
  PUBLIC ${INCLUDE_DIRS})
