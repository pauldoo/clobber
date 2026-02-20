cmake_minimum_required(VERSION 3.28)
project(clobber)

#set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(clobber)
set_property(TARGET clobber PROPERTY CXX_STANDARD 17)

target_compile_options(clobber PRIVATE -Wall -Wextra -fhardened -flto)
target_link_options(clobber PRIVATE -Wall -Wextra -fhardened -flto)

target_sources(clobber
  PRIVATE
    basic_types.cpp
    board.cpp
    game.cpp
    game_state.cpp
    human_player.cpp
    main.cpp
    player.cpp
    random.cpp
    random_player.cpp
)
