cmake_minimum_required(VERSION 3.11)

find_package(OpenGL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(SIGC++ REQUIRED)

set(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/")

set(MANIACLIB_HEADERS
  )

set(MANIACLIB_SRC
    logic/physics.cpp
    logic/level.cpp
    logic/stamp.cpp
    logic/game_object.cpp
    logic/explosion_object.cpp
    logic/bomb_object.cpp
    logic/movements.cpp
    logic/particles.cpp
    logic/wall_object.cpp
    logic/fog_object.cpp
    logic/fan_object.cpp
    logic/sensor.cpp
    logic/tileset.cpp
    logic/builtin_tileset.cpp

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
