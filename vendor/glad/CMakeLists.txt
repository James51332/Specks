cmake_minimum_required(VERSION 3.20)
project(Glad)

set(BUILD_SHARED_LIBS off)

message(VERBOSE "Building Shared GLAD Libs: " ${BUILD_SHARED_LIBS})

add_library(glad include/glad/glad.h src/glad.c)
target_include_directories(glad PUBLIC include/)