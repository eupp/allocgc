find_package(Threads REQUIRED)

set(boehm_SRC
        ../../common/macro.h
        boehm_test.cpp)

include_directories(${CMAKE_SOURCE_DIR}/precisegc/include)

set( CMAKE_VERBOSE_MAKEFILE on )

find_library(BDWGC NAMES gc
        PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        )

add_executable(boehm_test ${boehm_SRC})
target_link_libraries(boehm_test ${CMAKE_THREAD_LIBS_INIT})

option(NO_GC OFF)
option(BDW_GC OFF)
option(SHARED_PTR OFF)
option(PRECISE_GC OFF)

if(NO_GC)
    add_definitions(-DNO_GC)
endif()

if(BDW_GC)
    target_link_libraries(boehm_test ${BDWGC})
    add_definitions(-DBDW_GC)
endif()

if(SHARED_PTR)
    add_definitions(-DSHARED_PTR)
endif()

if(PRECISE_GC)
    target_link_libraries(boehm_test libprecisegc)
    add_definitions(-DPRECISE_GC)
endif()
