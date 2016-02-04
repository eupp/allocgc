find_package(Threads REQUIRED)

set(boehm_SRC
        boehm_test.cpp)

include_directories(${CMAKE_SOURCE_DIR}/precisegc/include)

add_executable(boehm_test ${boehm_SRC})
target_link_libraries(boehm_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(boehm_test libprecisegc)