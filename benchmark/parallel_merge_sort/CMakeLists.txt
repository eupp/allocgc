find_package(Threads REQUIRED)

set(par_merge_sort_SRC
        ../../common/macro.hpp
        ../../common/timer.hpp
        parallel_merge_sort.cpp)

include_directories(${CMAKE_SOURCE_DIR}/precisegc/include)

set( CMAKE_VERBOSE_MAKEFILE on )

find_library(BDWGC NAMES gc
        PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        )

add_executable(parallel_merge_sort ${par_merge_sort_SRC})
target_link_libraries(parallel_merge_sort ${CMAKE_THREAD_LIBS_INIT})

option(NO_GC OFF)
option(BDW_GC OFF)
option(SHARED_PTR OFF)
option(PRECISE_GC OFF)

#set(NO_GC ON)
#set(BDW_GC ON)
set(SHARED_PTR ON)
#set(PRECISE_GC ON)

if(NO_GC)
    add_definitions(-DNO_GC)
endif()

if(BDW_GC)
    target_link_libraries(parallel_merge_sort ${BDWGC})
    add_definitions(-DBDW_GC)
endif()

if(SHARED_PTR)
    add_definitions(-DSHARED_PTR)
endif()

if(PRECISE_GC)
    target_link_libraries(parallel_merge_sort libprecisegc)
    add_definitions(-DPRECISE_GC)
endif()
