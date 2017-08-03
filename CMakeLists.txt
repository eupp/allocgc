project(allocgc)
include(CheckIncludeFile)
include(CheckFunctionExists)
include (CheckLibraryExists)
cmake_minimum_required(VERSION 3.0.2)

macro(remove_cxx_release_flag flag)
    string(REPLACE "${flag}" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -fPIC -fno-strict-aliasing -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")

#remove_cxx_release_flag("-O3")

#set(CMAKE_VERBOSE_MAKEFILE on)

add_subdirectory(allocgc)
add_subdirectory(test)
add_subdirectory(benchmark/cord)
add_subdirectory(benchmark/grid)
add_subdirectory(benchmark/boehm)
add_subdirectory(benchmark/multisize_boehm)
add_subdirectory(benchmark/producer_consumer)
add_subdirectory(benchmark/parallel_merge_sort)
