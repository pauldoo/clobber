cmake_minimum_required(VERSION 3.28)
project(clobber)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SHARED_OPTIONS
    -flto
    -fno-fat-lto-objects
    -fhardened
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    -Wall
    -Wextra
    -Wpedantic
    -Wno-hardened
  )

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(${SHARED_OPTIONS})

  add_link_options(${SHARED_OPTIONS})
endif()

add_executable(clobber)
#target_link_options(clobber PRIVATE -static)

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
