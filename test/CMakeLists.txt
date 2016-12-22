enable_testing()
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})

set(precisegc_TESTS
        include/time_util.h
        include/memory_descriptor_mock.h
        include/test_chunk.h

        gc_test.cpp
        gc_inside_method_test.cpp
        details/gc_type_meta_factory_test.cpp
        details/gc_heap_test.cpp
        gc_new_test.cpp
        details/gc_untyped_ptr_test.cpp
        gc_environment.cpp
        details/index_tree_test.cpp
        details/allocators/pool_chunk_test.cpp
        details/allocators/bucket_allocator_test.cpp
        details/allocators/pow2_bucket_policy_test.cpp
        details/indexed_managed_object_test.cpp
        include/page_ptr.h
        details/managed_pool_chunk_test.cpp
        include/rand_util.h
        gc_ptr_test.cpp
        details/threads/pending_call_test.cpp
        details/threads/pin_set_test.cpp
        details/threads/signal_test.cpp
        details/threads/ass_sync_test.cpp
        details/threads/stw_manager_test.cpp
        details/threads/managed_thread_test.cpp
        details/threads/thread_manager_test.cpp

        details/utils/flattened_range_test.cpp
        details/utils/bitset_test.cpp
        details/garbage_collector_test.cpp
        include/serial_gc_mock.hpp
        include/initiation_policy_mock.hpp
        include/incremental_gc_mock.hpp
        details/space_based_policy_test.cpp
        details/utils/dynarray_test.cpp
        details/ptrs/trace_ptr_test.cpp
        details/utils/base_offset_test.cpp
        details/utils/static_thread_pool_test.cpp details/collectors/marker_test.cpp
        include/test_descriptor.hpp
        details/utils/barrier_test.cpp
        details/object_meta_test.cpp
        details/gc_handle_test.cpp
        details/allocators/allocators_test.cpp
        details/threads/pin_stack_test.cpp
        details/threads/stack_bitmap_test.cpp
        details/compacting/compactor_test.cpp
        include/test_forwarding.hpp
        details/compacting/fix_ptrs_test.cpp
        details/allocators/mlo_allocator_test.cpp
        details/allocators/mpool_allocator_test.cpp)

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