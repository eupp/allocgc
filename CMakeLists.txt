project(diploma)
include(CheckIncludeFile)
include(CheckFunctionExists)
include (CheckLibraryExists)
cmake_minimum_required(VERSION 3.0.2)

macro(remove_cxx_release_flag flag)
    string(REPLACE "${flag}" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endmacro()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -fPIC -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2")


remove_cxx_release_flag("-O3")

add_subdirectory(precisegc)
add_subdirectory(microbenchmark)
add_subdirectory(test)
add_subdirectory(benchmark/boehm)
add_subdirectory(benchmark/multisize_boehm)
add_subdirectory(benchmark/parallel_merge_sort)
