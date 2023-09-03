cmake_minimum_required(VERSION 3.20)

# Set where the ImGui files are stored
set(IMGUI_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Compile as static library 
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})