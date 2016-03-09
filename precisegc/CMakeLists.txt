
set(precisegc_LIB_HDRS
        include/libprecisegc/debug_print.h
        include/libprecisegc/fake_roots.h
        include/libprecisegc/gc_ptr.h
        include/libprecisegc/gc_new.h
        include/libprecisegc/gcmalloc.h
        include/libprecisegc/index_tree.h
        include/libprecisegc/stack.h
        include/libprecisegc/taginfo.h
        include/libprecisegc/tlvars.h
        include/libprecisegc/thread.h
        include/libprecisegc/meta_information.h
        include/libprecisegc/object.h

        include/libprecisegc/details/class_meta.h
        include/libprecisegc/details/mutex.h
        include/libprecisegc/details/os.h
        include/libprecisegc/details/page_descriptor.h
        include/libprecisegc/details/segregated_list.h
        include/libprecisegc/details/gc_heap.h
        include/libprecisegc/details/allocators/constants.h
        include/libprecisegc/details/os_mem_ptr.h
        include/libprecisegc/details/math_util.h
        include/libprecisegc/details/forwarding_list.h
        include/libprecisegc/details/iterator_facade.h
        include/libprecisegc/details/iterator_access.h
        include/libprecisegc/details/pool_chunk.h
        include/libprecisegc/details/condition_variable.h
        include/libprecisegc/details/gc_pause.h
        include/libprecisegc/details/barrier.h
        include/libprecisegc/details/thread_list.h
        include/libprecisegc/details/signal_safe_sync.h
        include/libprecisegc/details/util.h
        include/libprecisegc/details/gc_compact.h
        include/libprecisegc/details/gc_new_stack.h
        include/libprecisegc/details/gc_new_impl.h
        include/libprecisegc/details/gc_unsafe_scope.h
        include/libprecisegc/details/gc_untyped_ptr.h
        include/libprecisegc/details/gc_untyped_pin.h
        include/libprecisegc/details/gc_ptr_access.h
        include/libprecisegc/details/signal_lock.h
        include/libprecisegc/details/logging.h
        include/libprecisegc/gc_init.h
        include/libprecisegc/details/gc_mark_queue.h
        include/libprecisegc/details/gc_mark.h
        include/libprecisegc/details/gc_garbage_collector.h
        include/libprecisegc/details/write_barrier.h
        include/libprecisegc/gc.h
        include/libprecisegc/details/gc_initator.h
        include/libprecisegc/details/allocators/paged_allocator.h
        include/libprecisegc/details/allocators/index_tree.h
        include/libprecisegc/details/allocators/types.h
        include/libprecisegc/details/allocators/any_ptr.h
        include/libprecisegc/details/allocators/memory.h include/libprecisegc/details/allocators/debug_layer.h include/libprecisegc/details/allocators/bucket_allocator.h include/libprecisegc/details/allocators/plain_pool_chunk.h include/libprecisegc/details/allocators/fixed_size_allocator.h include/libprecisegc/details/allocators/stl_adapter.h include/libprecisegc/details/allocators/region.h include/libprecisegc/details/allocators/pointer_decorator.h include/libprecisegc/details/allocators/indexed_ptr.h include/libprecisegc/details/allocators/pow2_bucket_policy.h include/libprecisegc/details/managed_pool_chunk.h include/libprecisegc/details/managed_memory_descriptor.h include/libprecisegc/details/managed_ptr.h include/libprecisegc/details/allocators/iterator_range.h include/libprecisegc/details/allocators/lockable_range.h)

set(precisegc_LIB_SRCS
        src/index_tree.cpp
        src/gcmalloc.cpp
        src/gc_mark.cpp
        src/stack.cpp
        src/taginfo.cpp
        src/thread.cpp
        src/tlvars.cpp
        src/page_descriptor.cpp
        src/segregated_list.cpp
        src/gc_heap.cpp src/pool_chunk.cpp
        src/gc_pause.cpp
        src/thread_list.cpp
        src/signal_safe_sync.cpp
        src/gc_new_stack.cpp
        src/gc_untyped_ptr.cpp
        src/gc_untyped_pin.cpp
        src/logging.cpp
        src/gc_init.cpp
        src/gc_garbage_collector.cpp
        src/gc_mark_queue.cpp
        src/gc_initator.cpp
        src/details/allocators/memory.cpp src/details/allocators/plain_pool_chunk.cpp src/details/managed_pool_chunk.cpp src/details/managed_ptr.cpp)


include_directories(include/libprecisegc)
include_directories(include/libprecisegc/details)

add_library(libprecisegc SHARED ${precisegc_LIB_HDRS} ${precisegc_LIB_SRCS})

find_package(Threads REQUIRED)
target_link_libraries(libprecisegc "${CMAKE_THREAD_LIBS_INIT}")

set_target_properties(libprecisegc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS libprecisegc LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${precisegc_LIB_HDRS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libprecisegc)