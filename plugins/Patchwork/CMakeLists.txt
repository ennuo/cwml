cmake_minimum_required(VERSION 3.20)
project(Patchwork VERSION 1.0.0 LANGUAGES CXX ASM)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/**.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/code/cwml/include
    ${CMAKE_SOURCE_DIR}/code/CWLib/src
    ${CMAKE_SOURCE_DIR}/code/CoreLib/src
    ${CMAKE_SOURCE_DIR}/code/Ib/include
)

target_link_libraries(${PROJECT_NAME} CWLib)
target_precompile_headers(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/code/CoreLib/src/types.h")
include(cell/sprx/patchwork)