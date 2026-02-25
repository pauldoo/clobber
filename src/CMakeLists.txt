cmake_minimum_required(VERSION 3.28)
project(clobber)

#set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(clobber)
set_property(TARGET clobber PROPERTY CXX_STANDARD 17)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  target_compile_options(clobber PRIVATE
    -flto
    -fno-fat-lto-objects
    -fhardened
    -Wall
    -Wextra
    -Wpedantic
    -Wno-hardened
  )

  target_link_options(clobber PRIVATE
    -flto
    -fno-fat-lto-objects
    -fhardened
    -Wall
    -Wextra
    -Wpedantic
    -Wno-hardened
  )
endif()

target_sources(clobber
  PRIVATE
    basic_types.cpp
    board.cpp
    game.cpp
    game_state.cpp
    human_player.cpp
    main.cpp
    mcts_player.cpp
    null_io.cpp
    player.cpp
    random.cpp
    random_player.cpp
)
