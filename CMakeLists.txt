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

set(SCENEPATH data/blender)
set(TILESETPATH data/tileset)
set(BLENDER blender)
set(TILE_RENDER_BLEND_DEPS ${SCENEPATH}/library.blend)

function(add_tile_render BLENDFILE SCENE PNG)
  add_custom_command(
    OUTPUT "${TILESETPATH}/${PNG}.png"
    COMMAND "${BLENDER}" ARGS -b "${SCENEPATH}/${BLENDFILE}.blend" -S "${SCENE}" -o "${TILESETPATH}/${PNG}.png" -f 1
    COMMAND mv ARGS "${TILESETPATH}/${PNG}.png0001.png" "${TILESETPATH}/${PNG}.png"
    DEPENDS ${TILE_RENDER_BLEND_DEPS} "${SCENEPATH}/${BLENDFILE}.blend"
    COMMENT Build tile image "${TILESETPATH}/${PNG}")
endfunction(add_tile_render)

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
    "src/logic/PythonInterface.cpp"
    "src/logic/Particles.cpp"
    "src/logic/PlayerObject.cpp"
    "src/logic/TileMaterial.cpp"
    "src/logic/MetatextureView.cpp"
    "src/logic/WallObject.cpp"
    "src/logic/RockObject.cpp"
    "src/logic/BombObject.cpp"
    "src/logic/ExplosionObject.cpp"
    "src/logic/Weapon.cpp"
)

set(COMMON_SOURCES)

set(GAME_SOURCES
    ${COMMON_SOURCES}
    "src/game/Mode.cpp"
    "src/game/MainMenu.cpp"
    "src/game/Playground.cpp"
    "src/game/Application.cpp"
)

set(EDITOR_SOURCES
    ${COMMON_SOURCES}
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
    "src/editor/LevelEditor.cpp"
    "src/editor/LevelCollectionEditee.cpp"
    "src/editor/LevelCollectionOperations.cpp"
    "src/editor/LevelCollectionEditor.cpp"
    "src/editor/RootWindow.cpp"
)

include_directories(${PYENGINE_INCLUDE_DIRS})
include_directories(structstream)
include_directories("src")
add_definitions(${PYENGINE_DEFINITIONS})

add_library(ml ${LIB_SOURCES})
target_link_libraries(ml ${PYENGINE_LINK_TARGETS} structstream++)

add_tile_render(safewallsq0 Diffuse safewallsq0-diffuse)
add_tile_render(safewallsq0 Emission wall0-emission)
add_tile_render(safewallsq1 Diffuse safewallsq1-diffuse)
add_tile_render(safewallsq1 Emission wall1-emission)
add_tile_render(safewallsq3 Diffuse safewallsq3-diffuse)
add_tile_render(safewallsq3 Emission wall3-emission)
add_tile_render(safewallrd0 Diffuse safewallrd0-diffuse)
add_tile_render(safewallrd1 Diffuse safewallrd1-diffuse)
add_tile_render(safewallrd3 Diffuse safewallrd3-diffuse)
add_tile_render(safewall5 Diffuse safewall5-diffuse)
add_tile_render(safewall5 Emission wall5-emission)
add_tile_render(safewall7 Diffuse safewall7-diffuse)
add_tile_render(safewall7 Emission wall7-emission)
add_tile_render(safewallf Diffuse safewallf-diffuse)
add_tile_render(safewallf Emission wallf-emission)
add_tile_render(rock Diffuse rock-diffuse)
add_tile_render(player Diffuse player-diffuse)
add_tile_render(player Emission player-emission)
add_tile_render(bomb Diffuse bomb-diffuse)

add_custom_target(
  render-tiles
  DEPENDS ${TILESETPATH}/safewallsq0-diffuse.png
  DEPENDS ${TILESETPATH}/safewallsq1-diffuse.png
  DEPENDS ${TILESETPATH}/safewallsq3-diffuse.png
  DEPENDS ${TILESETPATH}/safewallrd0-diffuse.png
  DEPENDS ${TILESETPATH}/safewallrd1-diffuse.png
  DEPENDS ${TILESETPATH}/safewallrd3-diffuse.png
  DEPENDS ${TILESETPATH}/safewall5-diffuse.png
  DEPENDS ${TILESETPATH}/safewall7-diffuse.png
  DEPENDS ${TILESETPATH}/safewallf-diffuse.png
  DEPENDS ${TILESETPATH}/wall0-emission.png
  DEPENDS ${TILESETPATH}/wall1-emission.png
  DEPENDS ${TILESETPATH}/wall3-emission.png
  DEPENDS ${TILESETPATH}/wall5-emission.png
  DEPENDS ${TILESETPATH}/wall7-emission.png
  DEPENDS ${TILESETPATH}/wallf-emission.png
  DEPENDS ${TILESETPATH}/rock-diffuse.png
  DEPENDS ${TILESETPATH}/player-diffuse.png
  DEPENDS ${TILESETPATH}/player-emission.png
  DEPENDS ${TILESETPATH}/bomb-diffuse.png
  )

add_custom_target(
  compose-tiles
  convert -size 1024x1024 xc:none ${TILESETPATH}/atlas.png
  COMMAND composite -geometry       +0+0 ${TILESETPATH}/player-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +128+0 ${TILESETPATH}/safewallsq0-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +192+0 ${TILESETPATH}/safewallsq1-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +256+0 ${TILESETPATH}/safewallsq3-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +320+0 ${TILESETPATH}/safewall5-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +384+0 ${TILESETPATH}/safewall7-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +448+0 ${TILESETPATH}/safewallf-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry    +128+64 ${TILESETPATH}/rock-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry    +192+64 ${TILESETPATH}/safewallrd0-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry    +256+64 ${TILESETPATH}/safewallrd1-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry    +320+64 ${TILESETPATH}/safewallrd3-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry    +384+64 ${TILESETPATH}/bomb-diffuse.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png

  COMMAND composite -geometry     +0+512 ${TILESETPATH}/wallf-emission.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry   +192+512 ${TILESETPATH}/wall0-emission.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry   +384+512 ${TILESETPATH}/wall1-emission.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +0+704 ${TILESETPATH}/wall3-emission.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry   +192+704 ${TILESETPATH}/wall5-emission.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry   +384+704 ${TILESETPATH}/wall7-emission.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  COMMAND composite -geometry     +0+896 ${TILESETPATH}/player-emission.png ${TILESETPATH}/atlas.png ${TILESETPATH}/atlas.png
  )
add_dependencies(compose-tiles render-tiles)

add_executable(ml-game "src/game/maniaclab.cpp" ${GAME_SOURCES})
add_dependencies(ml-game ${PYENGINE_DEPENDENCIES} structstream++ ml compose-tiles)
target_link_libraries(ml-game ${PYENGINE_LINK_TARGETS} "pthread" structstream++ ml)

include_directories(${GTKMM_INCLUDE_DIRS})

add_executable(ml-edit "src/editor/ml-edit.cpp" ${EDITOR_SOURCES})
add_dependencies(ml-edit structstream++ ml compose-tiles)
target_link_libraries(ml-edit
    "pthread"
    structstream++
    ${GTKMM_LIBRARIES}
    ${PYENGINE_LINK_TARGETS}
    ml)
