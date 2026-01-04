#!/bin/cmake .

cmake_minimum_required(VERSION 3.30)

set(PROJECT_NAME smake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(BINARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIRECTORY})

project(${PROJECT_NAME} CXX)

add_executable(${PROJECT_NAME}
    src/main.cpp

    src/exec.cpp
    src/parser.cpp
    src/project_config.cpp

    # internal
    src/util.cpp
)

include_directories(include)
include_directories(src/include)

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)
