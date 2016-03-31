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
target_link_libraries(boehm_test ${BDWGC})
target_link_libraries(boehm_test libprecisegc)
