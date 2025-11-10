cmake_minimum_required(VERSION 3.5)
project(loader LANGUAGES C)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic ")

add_executable(main
   src/main.c
   src/filesystem.c
   src/command_parser/command_parser.c
   src/help_functions.c

)

target_link_libraries(main m)