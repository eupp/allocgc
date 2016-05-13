enable_testing()
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})

set(precisegc_TESTS
        include/time_util.h
        include/memory_descriptor_mock.h
        include/test_chunk.h

        gc_test.cpp
        gc_inside_method_test.cpp
        details/class_meta_test.cpp
        details/gc_pause_test.cpp
        details/signal_safe_sync_test.cpp
        details/gc_heap_test.cpp
        details/gc_compact_test.cpp
        gc_new_test.cpp
        details/gc_untyped_ptr_test.cpp
        gc_environment.cpp
        details/allocators/index_tree_test.cpp
        details/allocators/plain_pool_chunk_test.cpp
        details/allocators/fixed_size_allocator_test.cpp
        details/allocators/bucket_allocator_test.cpp
        details/allocators/indexed_ptr_test.cpp
        details/allocators/pow2_bucket_policy_test.cpp
        details/managed_ptr_test.cpp
        include/page_ptr.h details/managed_pool_chunk_test.cpp include/rand_util.h details/allocators/join_range_test.cpp
        gc_ptr_test.cpp details/threads/pending_call_test.cpp details/root_set_test.cpp details/threads/signal_test.cpp details/threads/ass_sync_test.cpp details/threads/stw_manager_test.cpp details/threads/managed_thread_test.cpp details/threads/thread_manager_test.cpp)

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/precisegc/include)

find_package(Threads REQUIRED)

find_library(GMOCK_LIB NAMES libgmock.a
        PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        )

find_library(GMOCK_MAIN_LIB NAMES libgmock_main.a
        PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        )


add_executable(precisegc_test ${precisegc_TESTS})
target_link_libraries(precisegc_test ${GMOCK_LIB})
target_link_libraries(precisegc_test ${GMOCK_MAIN_LIB})
target_link_libraries(precisegc_test ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(precisegc_test libprecisegc)

add_test(all_tests precisegc_test)

add_custom_target(precisegc_test_memcheck
        COMMAND valgrind --leak-check=full --leak-resolution=med ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/precisegc_test
        DEPENDS precisegc_test)

add_custom_target(precisegc_test_helgrind
        COMMAND valgrind --tool=helgrind ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/precisegc_test
        DEPENDS precisegc_test)