cmake_minimum_required(VERSION 3.10)

project(Particles)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(ParticleSim src/main.cpp)

target_include_directories(ParticleSim PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(ParticleSim PUBLIC ${OPENGL_INCLUDE_DIR})

target_link_libraries(ParticleSim ${SDL2_LIBRARIES})
target_link_libraries(ParticleSim ${OPENGL_LIBRARIES})