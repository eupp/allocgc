find_package(Threads REQUIRED)

set(boehm_SRC
        ../../common/macro.hpp
        ../../common/timer.hpp
        boehm.cpp)

include_directories(${CMAKE_SOURCE_DIR}/allocgc/include)

set( CMAKE_VERBOSE_MAKEFILE on )

find_library(BDWGC NAMES gc
        PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        )

add_executable(boehm ${boehm_SRC})
target_link_libraries(boehm ${CMAKE_THREAD_LIBS_INIT})

option(NO_GC OFF)
option(BDW_GC OFF)
option(UNIQUE_PTR OFF)
option(SHARED_PTR OFF)
option(PRECISE_GC_SERIAL OFF)
option(PRECISE_GC_CMS OFF)

#set(NO_GC ON)
#set(BDW_GC ON)
#set(UNIQUE_PTR ON)
#set(SHARED_PTR ON)
#set(PRECISE_GC_SERIAL ON)
set(PRECISE_GC_CMS ON)

if(NO_GC)
    add_definitions(-DNO_GC)
endif()

if(BDW_GC)
    target_link_libraries(boehm ${BDWGC})
    add_definitions(-DBDW_GC)
endif()

if(UNIQUE_PTR)
    add_definitions(-DUNIQUE_PTR)
endif()

if(SHARED_PTR)
    add_definitions(-DSHARED_PTR)
endif()

if(PRECISE_GC_SERIAL)
    target_link_libraries(boehm liballocgc)
    add_definitions(-DPRECISE_GC_SERIAL)
endif()

if(PRECISE_GC_CMS)
    target_link_libraries(boehm liballocgc)
    add_definitions(-DPRECISE_GC_CMS)
endif()