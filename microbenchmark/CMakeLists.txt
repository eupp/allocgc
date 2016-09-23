set(precisegc_microbenchmark
        main.cpp
        index_tree_benchmark.cpp
        pointers_benchmark.cpp
        ../common/macro.hpp stack_map_benchmark.cpp
        allocators_benchmark.cpp
        deoptimize.hpp
        pool_chunks_benchmark.cpp
        math_benchmark.cpp
        approx_stack_map_benchmark.cpp)

include_directories(${CMAKE_SOURCE_DIR}/precisegc/include)

find_package(Threads REQUIRED)
find_package(Boost 1.54 COMPONENTS system chrono)

find_library(BDWGC NAMES gc
        PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        )

add_definitions(-DNONIUS_USE_BOOST_CHRONO)

add_executable(precisegc_microbenchmark ${precisegc_microbenchmark})
target_link_libraries(precisegc_microbenchmark ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(precisegc_microbenchmark ${Boost_LIBRARIES})
target_link_libraries(precisegc_microbenchmark ${BDWGC})
target_link_libraries(precisegc_microbenchmark libprecisegc)
