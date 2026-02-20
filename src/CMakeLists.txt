cmake_minimum_required(VERSION 3.28)
project(clobber)

#set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(clobber)
set_property(TARGET clobber PROPERTY CXX_STANDARD 17)

target_compile_options(clobber PRIVATE -Wall -Wextra -fhardened)
target_link_options(clobber PRIVATE -Wall -Wextra -fhardened)

target_sources(clobber
  PRIVATE
    main.cpp
)
