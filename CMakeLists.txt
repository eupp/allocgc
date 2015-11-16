project(diploma)
include(CheckIncludeFile)
include(CheckFunctionExists)
include (CheckLibraryExists)
cmake_minimum_required(VERSION 3.0.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2")

CHECK_LIBRARY_EXISTS(m main "" HAVE_LIBM)

CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(sys/param.h HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)

CHECK_FUNCTION_EXISTS(malloc HAVE_MALLOC)
CHECK_FUNCTION_EXISTS(mmap HAVE_MMAP)
CHECK_FUNCTION_EXISTS(memset HAVE_MEMSET)
CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

add_subdirectory(precisegc)
add_subdirectory(test)

#test
#file(GLOB children "test/*")
#foreach(child ${children})
#    if(IS_DIRECTORY ${child} AND EXISTS "${child}/CMakeLists.txt")
#        message("Found test folder: ${child}")
#        add_subdirectory(${child})
#    endif()
#endforeach()
#
#add_custom_target(test
#    DEPENDS gc_inside_method
#            multi_lists
#            binary_tree_sort
#            concurrency
#            mark-and-copy-test
#            boehm
#            gc_ptr_timing
#            skiplist
#            merge_sort)
