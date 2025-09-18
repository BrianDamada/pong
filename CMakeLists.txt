cmake_minimum_required(VERSION 3.10)

project(MySimpleApp VERSION 1.0 LANGUAGES CXX)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(pong
    main.cpp
)

target_link_libraries(
    pong
    PRIVATE SDL2::SDL2
)