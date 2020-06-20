cmake_minimum_required(VERSION 3.1)

project(headerlibs)

link_libraries("m")


if(UNIX)
message("Unix detected")
add_compile_options(-Wall -Wextra -Werror)
endif()

add_executable(test test.c)