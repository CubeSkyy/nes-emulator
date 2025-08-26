cmake_minimum_required(VERSION 3.31)
project(imgui)

file(GLOB IMGUI_SOURCES *.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})