cmake_minimum_required(VERSION 3.29)
project(linux-chat C)

set(CMAKE_C_STANDARD 11)

add_executable(c client.c c_command.h c_command.c macros.h utils.c utils.h)
add_executable(s server.c macros.h utils.c utils.h)

