# Basic Info
cmake_minimum_required(VERSION 3.8)

# Set the C++ Standard
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED True)

project(Specks)

# Vendored SDL Library (Output to the same directory as project)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(vendor/sdl2 EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glad EXCLUDE_FROM_ALL)
add_subdirectory(vendor/glm EXCLUDE_FROM_ALL)

# Source Files
file(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.h")

# Define the include directory for the program
include_directories(Specks ${CMAKE_SOURCE_DIR}/src)

# Define the executable for the program
add_executable(Specks ${SRC_FILES})

# Link to the SDL library
target_link_libraries(Specks PRIVATE SDL3::SDL3 glad glm::glm)
