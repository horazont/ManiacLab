find_package(SIGC++ REQUIRED)

set(TEST_SRC
    main.cpp
    logic/sensor.cpp
    )

add_executable(tests ${TEST_SRC})
setup_scc_target(tests)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Catch/include)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../game)
target_link_libraries(tests ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(tests maniaclib ffengine-render ffengine-sim ffengine-core)
target_link_libraries(tests sigc++)
