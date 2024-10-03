cmake_minimum_required(VERSION 3.12)

project(program2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
    ${CMAKE_SOURCE_DIR}/deps/glm
    ${CMAKE_SOURCE_DIR}/deps/glfw/include
    ${CMAKE_SOURCE_DIR}/deps/glew/include
    ${CMAKE_SOURCE_DIR}/deps/stb
)

link_directories(
    ${CMAKE_SOURCE_DIR}/deps/glfw/lib-vc2019
    ${CMAKE_SOURCE_DIR}/deps/glew/lib/Release/x64
)

add_executable(program2 main.cpp)

target_link_libraries(program2 PRIVATE glfw3.lib glew32.lib opengl32.lib)

add_custom_command(TARGET program2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/deps/glew/bin/Release/x64/glew32.dll"
    $<TARGET_FILE_DIR:program2>)