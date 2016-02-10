enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Threads REQUIRED)

set(precisegc_TESTS
        include/time_util.h
        gc_test.cpp
        gc_inside_method_test.cpp
        details/class_meta_test.cpp
        details/page_descriptor_test.cpp
        details/segregated_list_test.cpp
        details/pool_chunk_test.cpp
        details/gc_pause_test.cpp
        details/signal_safe_sync_test.cpp
        details/gc_heap_test.cpp
        details/gc_compact_test.cpp
        gc_new_test.cpp
        details/gc_untyped_ptr_test.cpp)

include_directories(include)
include_directories(../../precisegc/include)

add_executable(precisegc_test ${precisegc_TESTS})
target_link_libraries(precisegc_test ${GTEST_BOTH_LIBRARIES})
target_link_libraries(precisegc_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(precisegc_test libprecisegc)

add_test(all_tests precisegc_test)

add_custom_target(precisegc_test_memcheck
        COMMAND valgrind --leak-check=full --leak-resolution=med ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/precisegc_test
        DEPENDS precisegc_test)

add_custom_target(precisegc_test_helgrind
        COMMAND valgrind --tool=helgrind ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/precisegc_test
        DEPENDS precisegc_test)