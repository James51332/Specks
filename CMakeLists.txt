cmake_minimum_required(VERSION 3.20)

# Basic Info
project(Specks)

# Set the C++ Standard
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED True)

# Source Files
file(GLOB_RECURSE SRC_FILES CMAKE_CONFIGURE_DEPENDS "src/*.cpp" "src/*.h src/**.cpp src/**.h")

# Define the executable for the program
add_executable(Specks ${SRC_FILES})

target_include_directories(Specks PRIVATE "src")

add_subdirectory(vendor/vision)

# Link to the SDL library
target_link_libraries(Specks 
                        PUBLIC
                          Vision)