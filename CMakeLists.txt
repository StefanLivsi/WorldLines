cmake_minimum_required(VERSION 3.20)
project(WorldLines)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
find_package(OpenMP)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS "${CmakeLists.txt} -O2 -march=native")


set(CMAKE_CXX_STANDARD 17)
add_executable(WorldLines src/main.cpp)
set(SOURCE_FILES src/main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
add_library(WL src/WL.cpp h/WL.h)
target_include_directories(WorldLines PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/h"
        )
#and here
target_link_libraries( ${PROJECT_NAME} WL )
