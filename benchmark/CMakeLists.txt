set(precisegc_BENCHMARKS
        main.cpp
        signal_lock_benchmark.cpp
        parallel_merge_sort_benchmark.cpp index_tree_benchmark.cpp pointers_benchmark.cpp)

include_directories(${CMAKE_SOURCE_DIR}/precisegc/include)

find_package(Threads REQUIRED)
find_package(Boost 1.54 COMPONENTS system chrono)

add_definitions(-DNONIUS_USE_BOOST_CHRONO)

add_executable(precisegc_benchmark ${precisegc_BENCHMARKS})
target_link_libraries(precisegc_benchmark ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(precisegc_benchmark ${Boost_LIBRARIES})
target_link_libraries(precisegc_benchmark libprecisegc)