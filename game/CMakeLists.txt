cmake_minimum_required(VERSION 3.11)

set(CMAKE_INCLUDE_CURRENT_DIRS ON)
set(CMAKE_AUTOMOC ON)

set(OpenGL_GL_PREFERENCE "GLVND")

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
  materials.cpp
  intro/intro.cpp
  intro/scene.cpp
  editor/editor.cpp
  editor/tools.cpp
  editor/drag.cpp
  )

set(GAME_HEADERS
  fixups.hpp
  application.hpp
  mainmenu.hpp
  mode.hpp
  openglscene.hpp
  actionbutton.hpp
  ingame.hpp
  materials.hpp
  intro/intro.hpp
  intro/scene.hpp
  editor/editor.hpp
  editor/tools.hpp
  editor/drag.hpp
  )

set(GAME_UIS
    ui/ingame.ui
    ui/intro.ui
    ui/application.ui
    ui/openglscene.ui
    ui/mainmenu.ui
    ui/editor.ui)

QT5_WRAP_UI(GAME_UI_HDRS ${GAME_UIS})

set(GAME_TEXTURE_RESOURCES
  )

set(GAME_SHADER_RESOURCES
    resources/shaders/debug/physics.vert
    resources/shaders/debug/physics.frag
    resources/shaders/debug/physics_flow.vert
    resources/shaders/debug/physics_flow.frag
    resources/shaders/debug/physics_particle.vert
    resources/shaders/debug/physics_particle.frag
    resources/shaders/editor/grid.vert
    resources/shaders/editor/grid.frag
    resources/shaders/effects/fullscreenquad.vert
    resources/shaders/effects/ghostplane.frag
    resources/shaders/effects/lightcone.frag
    resources/shaders/effects/gameplane.vert
    resources/shaders/effects/fog.frag
    resources/shaders/intro/simple.vert
    resources/shaders/intro/simple.frag
    resources/shaders/intro/lightning-effect.frag
    resources/shaders/intro/mixable.frag
    resources/shaders/intro/rain-overlay.frag
    resources/shaders/lib/matrix_block.glsl
    resources/shaders/lib/inv_matrix_block.glsl
    resources/shaders/lib/model_matrix_block.glsl
    resources/shaders/lib/unproject.glsl
    resources/shaders/objects/object.vert
    resources/shaders/objects/dirt.frag
    resources/shaders/objects/fallback.frag
    resources/shaders/objects/simple.frag
  )

set(GAME_STYLESHEET_RESOURCES
  )

set(GAME_TILESET_CORE_RESOURCES
    resources/tileset/wall_pillar_v.png
    resources/tileset/wall_pillar_h.png
    resources/tileset/player.png
    resources/tileset/bg4.png
    resources/tileset/fog_noise.png
    resources/tileset/dirt.png
    resources/tileset/rock.png
    )

set(GAME_INTRO_RESOURCES
    resources/intro/c/lightcone.png
    resources/intro/s0/i0.png
    resources/intro/s0/i1.png
    resources/intro/s0/lightning-mask.png
    resources/intro/s1/bg.png
    resources/intro/s1/lcat-head.png
    resources/intro/s1/lcat-tail.png
    resources/intro/s1/lcat-torso.png
    resources/intro/s1/rcat-head.png
    resources/intro/s1/rcat-tail.png
    resources/intro/s1/rcat-torso.png
    )

# set(SCENEPATH resources/blender)
# set(TILESETPATH resources/tileset/core)
# set(BLENDER blender)
# set(TILE_RENDER_BLEND_DEPS
#     ${SCENEPATH}/library.blend
#     ${SCENEPATH}/explosive.png)

# function(add_tile_render BLENDFILE SCENE PNG)
#   add_custom_command(
#     OUTPUT "${TILESETPATH}/${PNG}.png"
#     COMMAND "${BLENDER}" ARGS -b "${CMAKE_CURRENT_SOURCE_DIR}/${SCENEPATH}/${BLENDFILE}.blend" -S "${SCENE}" -o "${TILESETPATH}/${PNG}.png" -f 1
#     COMMAND mv ARGS "${TILESETPATH}/${PNG}.png0001.png" "${TILESETPATH}/${PNG}.png"
#     DEPENDS ${TILE_RENDER_BLEND_DEPS} "${SCENEPATH}/${BLENDFILE}.blend"
#     COMMENT Build tile image "${TILESETPATH}/${PNG}")
# endfunction(add_tile_render)

# add_tile_render(safewallsq0 Diffuse safewallsq0-diffuse)
# add_tile_render(safewallsq0 Emission wall0-emission)
# add_tile_render(safewallsq1 Diffuse safewallsq1-diffuse)
# add_tile_render(safewallsq1 Emission wall1-emission)
# add_tile_render(safewallsq3 Diffuse safewallsq3-diffuse)
# add_tile_render(safewallsq3 Emission wall3-emission)
# add_tile_render(safewallrd0 Diffuse safewallrd0-diffuse)
# add_tile_render(safewallrd1 Diffuse safewallrd1-diffuse)
# add_tile_render(safewallrd3 Diffuse safewallrd3-diffuse)
# add_tile_render(safewall5 Diffuse safewall5-diffuse)
# add_tile_render(safewall5 Emission wall5-emission)
# add_tile_render(safewall7 Diffuse safewall7-diffuse)
# add_tile_render(safewall7 Emission wall7-emission)
# add_tile_render(safewallf Diffuse safewallf-diffuse)
# add_tile_render(safewallf Emission wallf-emission)
# add_tile_render(rock Diffuse rock-diffuse)
# add_tile_render(player Diffuse player-diffuse)
# add_tile_render(player Emission player-emission)
# add_tile_render(bomb Diffuse bomb-diffuse)

add_custom_target(
    game-resources
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/shaders.rcc
    ${CMAKE_CURRENT_BINARY_DIR}/textures.rcc
    ${CMAKE_CURRENT_BINARY_DIR}/stylesheets.rcc
    ${CMAKE_CURRENT_BINARY_DIR}/tileset-core.rcc
    ${CMAKE_CURRENT_BINARY_DIR}/intro.rcc
    SOURCES
    resources/shaders.qrc
    ${GAME_SHADER_RESOURCES}
    resources/textures.qrc
    ${GAME_TEXTURE_RESOURCES}
    resources/stylesheets.qrc
    ${GAME_STYLESHEET_RESOURCES}
    resources/tileset-core.qrc
    ${GAME_TILESET_CORE_RESOURCES}
    resources/intro.qrc
    ${GAME_INTRO_RESOURCES}
    ${GAME_UIS})

function(add_resource_pack PACKNAME QRC SOURCES)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKNAME}.rcc
    COMMAND rcc "${CMAKE_CURRENT_SOURCE_DIR}/${QRC}" --binary -o "${CMAKE_CURRENT_BINARY_DIR}/${PACKNAME}.rcc"
    DEPENDS ${SOURCES} ${QRC}
    COMMENT "Pack ${PACKNAME} resources"
  )
endfunction(add_resource_pack)

function(add_built_resource_pack PACKNAME QRC SOURCES)
  add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${QRC}
   COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/${QRC}" "${CMAKE_CURRENT_BINARY_DIR}/${QRC}"
   DEPENDS ${QRC}
   COMMENT "Copy ${PACKNAME} QRC file to binary dir"
   )
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKNAME}.rcc
    COMMAND rcc "${CMAKE_CURRENT_BINARY_DIR}/${QRC}" --binary -o "${CMAKE_CURRENT_BINARY_DIR}/${PACKNAME}.rcc"
    DEPENDS ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${QRC}
    COMMENT "Pack ${PACKNAME} resources"
  )
endfunction(add_built_resource_pack)

add_resource_pack("textures" "resources/textures.qrc" "${GAME_TEXTURE_RESOURCES}")
add_resource_pack("shaders" "resources/shaders.qrc" "${GAME_SHADER_RESOURCES}")
add_resource_pack("stylesheets" "resources/stylesheets.qrc" "${GAME_STYLESHEET_RESOURCES}")
add_resource_pack("tileset-core" "resources/tileset-core.qrc" "${GAME_TILESET_CORE_RESOURCES}")
add_resource_pack("intro" "resources/intro.qrc" "${GAME_INTRO_RESOURCES}")

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
target_link_libraries(game maniaclib)
target_link_libraries(game ${SIGC++_LIBRARIES})
target_link_libraries(game ${PROTOBUF_LIBRARIES})
target_link_libraries(game ${OPENGL_LIBRARIES})
target_link_libraries(game epoxy)
target_link_libraries(game "atomic")

add_dependencies(game game-resources)
