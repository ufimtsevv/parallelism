cmake_minimum_required(VERSION 3.10)
project(SinSum CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_FLOAT "use float type" OFF)

if(USE_FLOAT)
    add_definitions(-DUSE_FLOAT)
endif()

add_executable(sin_sum main.cpp)

set_target_properties(sin_sum PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)