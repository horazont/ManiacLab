cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_INCLUDE_CURRENT_DIRS ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)
find_package(OpenGL REQUIRED)
find_package(epoxy REQUIRED)
find_package(SIGC++ REQUIRED)
find_package(Protobuf REQUIRED)

set(GAME_SRC
  application.cpp
  main.cpp
  mainmenu.cpp
  mode.cpp
  openglscene.cpp
  actionbutton.cpp
  ingame.cpp

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

  io/tileset_data.cpp
  )

set(GAME_HEADERS
  fixups.hpp
  )

set(GAME_UIS
    ui/ingame.ui
    ui/application.ui
    ui/openglscene.ui
    ui/mainmenu.ui)

QT5_WRAP_UI(GAME_UI_HDRS ${GAME_UIS})

set(GAME_TEXTURE_RESOURCES
  )

set(GAME_SHADER_RESOURCES
    resources/shaders/debug/physics.vert
    resources/shaders/debug/physics.frag
    resources/shaders/lib/matrix_block.glsl
    resources/shaders/lib/inv_matrix_block.glsl
    resources/shaders/lib/unproject.glsl
  )

set(GAME_STYLESHEET_RESOURCES
  )

add_custom_target(
    game-resources
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/shaders.rcc
    ${CMAKE_CURRENT_BINARY_DIR}/textures.rcc
    ${CMAKE_CURRENT_BINARY_DIR}/stylesheets.rcc
    SOURCES
    resources/shaders.qrc
    ${GAME_SHADER_RESOURCES}
    resources/textures.qrc
    ${GAME_TEXTURE_RESOURCES}
    resources/stylesheets.qrc
    ${GAME_STYLESHEET_RESOURCES}
    ${GAME_UIS})

function(add_resource_pack PACKNAME QRC SOURCES)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKNAME}.rcc
    COMMAND rcc "${CMAKE_CURRENT_SOURCE_DIR}/${QRC}" --binary -o "${CMAKE_CURRENT_BINARY_DIR}/${PACKNAME}.rcc"
    DEPENDS ${SOURCES} ${QRC}
    COMMENT "Pack ${PACKNAME} resources"
  )
endfunction(add_resource_pack)

add_resource_pack("textures" "resources/textures.qrc" "${GAME_TEXTURE_RESOURCES}")
add_resource_pack("shaders" "resources/shaders.qrc" "${GAME_SHADER_RESOURCES}")
add_resource_pack("stylesheets" "resources/stylesheets.qrc" "${GAME_STYLESHEET_RESOURCES}")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${PROTOBUF_INCLUDE_DIRS})

add_executable(game ${GAME_SRC} ${GAME_HEADERS} ${PROTO_SRCS} ${PROTO_HDRS} ${GAME_UI_HDRS})
setup_scc_target(game)

target_link_libraries(game Qt5::Widgets)
target_link_libraries(game ffengine-core)
target_link_libraries(game ffengine-render)
target_link_libraries(game ffengine-sim)
target_link_libraries(game sigc++)
target_link_libraries(game ${PROTOBUF_LIBRARIES})
target_link_libraries(game ${OPENGL_LIBRARIES})
target_link_libraries(game epoxy)
target_link_libraries(game "atomic")

add_dependencies(game game-resources)
