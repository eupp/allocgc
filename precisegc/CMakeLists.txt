
set(precisegc_LIB_HDRS
        include/libprecisegc/gc_ptr.hpp
        include/libprecisegc/gc_new.hpp
        include/libprecisegc/gc.hpp
        include/libprecisegc/gc_init_options.hpp
        include/libprecisegc/libprecisegc.hpp

        include/libprecisegc/details/type_meta.hpp
        include/libprecisegc/details/object_meta.hpp
        include/libprecisegc/details/gc_heap.hpp
        include/libprecisegc/details/constants.hpp
        include/libprecisegc/details/utils/math.hpp
        include/libprecisegc/details/forwarding.hpp
        include/libprecisegc/details/utils/barrier.hpp
        include/libprecisegc/details/utils/utility.hpp
        include/libprecisegc/details/gc_compact.hpp
        include/libprecisegc/details/threads/gc_new_stack.hpp
        include/libprecisegc/details/gc_unsafe_scope.hpp
        include/libprecisegc/details/ptrs/gc_untyped_ptr.hpp
        include/libprecisegc/details/logging.hpp
        include/libprecisegc/details/gc_mark.hpp
        include/libprecisegc/details/allocators/core_allocator.hpp
        include/libprecisegc/details/index_tree.hpp
        include/libprecisegc/details/types.hpp
        include/libprecisegc/details/allocators/debug_layer.hpp
        include/libprecisegc/details/allocators/bucket_allocator.hpp
        include/libprecisegc/details/allocators/bitmap_pool_chunk.hpp
        include/libprecisegc/details/allocators/list_allocator.hpp
        include/libprecisegc/details/allocators/stl_adapter.hpp
        include/libprecisegc/details/allocators/pow2_bucket_policy.hpp
        include/libprecisegc/details/allocators/managed_pool_chunk.hpp
        include/libprecisegc/details/memory_descriptor.hpp
        include/libprecisegc/details/managed_ptr.hpp
        include/libprecisegc/details/utils/flatten_range.hpp
        include/libprecisegc/details/threads/pending_call.hpp
        include/libprecisegc/details/threads/posix_signal.hpp
        include/libprecisegc/details/threads/managed_thread.hpp
        include/libprecisegc/details/threads/pin_set.hpp
        include/libprecisegc/details/threads/ass_sync.hpp
        include/libprecisegc/details/gc_exception.hpp
        include/libprecisegc/details/threads/thread_manager.hpp
        include/libprecisegc/details/threads/stw_manager.hpp
        include/libprecisegc/details/utils/scoped_thread.hpp
        include/libprecisegc/details/utils/scope_guard.hpp
        include/libprecisegc/details/threads/posix_thread.hpp
        include/libprecisegc/details/garbage_collector.hpp
        include/libprecisegc/details/collectors/marker.hpp
        include/libprecisegc/details/utils/locked_range.hpp
        include/libprecisegc/details/ptrs/trace_ptr.hpp
        include/libprecisegc/details/utils/system_error.hpp
        include/libprecisegc/details/gc_strategy.hpp
        include/libprecisegc/details/utils/to_string.hpp
        include/libprecisegc/details/utils/make_reverse_iterator.hpp
        include/libprecisegc/details/allocators/allocator_tag.hpp
        include/libprecisegc/details/utils/bitset.hpp
        include/libprecisegc/details/initiation_policy.hpp
        include/libprecisegc/details/collectors/serial_gc.hpp
        include/libprecisegc/details/threads/world_snapshot.hpp
        include/libprecisegc/details/utils/make_unique.hpp
        include/libprecisegc/details/collectors/incremental_gc.hpp
        include/libprecisegc/details/gc_clock.hpp
        include/libprecisegc/details/gc_interface.hpp
        include/libprecisegc/details/allocators/default_allocator.hpp
        include/libprecisegc/details/utils/dynarray.hpp
        include/libprecisegc/details/allocators/freelist_pool_chunk.hpp
        include/libprecisegc/details/allocators/intrusive_list_allocator.hpp
        include/libprecisegc/gc_pin.hpp
        include/libprecisegc/gc_ref.hpp
        include/libprecisegc/details/gc_handle.hpp
        include/libprecisegc/details/gc_hooks.hpp
        include/libprecisegc/details/utils/base_offset.hpp
        include/libprecisegc/details/gc_factory.hpp
        include/libprecisegc/gc_stat.hpp include/libprecisegc/details/utils/static_thread_pool.hpp
        include/libprecisegc/details/utils/dummy_mutex.hpp
        include/libprecisegc/details/collectors/packet_manager.hpp
        include/libprecisegc/details/utils/safepoint_lock.hpp
        include/libprecisegc/details/threads/this_managed_thread.hpp
        include/libprecisegc/details/threads/managed_thread_accessor.hpp
        include/libprecisegc/details/allocators/intrusive_list_allocator.hpp
        include/libprecisegc/details/allocators/managed_object_descriptor.hpp
        include/libprecisegc/details/gc_manager.hpp
        include/libprecisegc/details/utils/block_ptr.hpp
        include/libprecisegc/details/allocators/sys_allocator.hpp
        include/libprecisegc/details/allocators/cache_policies.hpp
        include/libprecisegc/details/allocators/managed_ptr_iterator.hpp
        include/libprecisegc/details/allocators/freelist_allocator.hpp
        include/libprecisegc/details/utils/get_ptr.hpp
        include/libprecisegc/details/allocators/pool.hpp
        include/libprecisegc/details/allocators/stack_chunk.hpp
        include/libprecisegc/details/allocators/sys_allocator.hpp
        include/libprecisegc/details/allocators/null_allocator.hpp
        include/libprecisegc/details/threads/pin_stack.hpp
        include/libprecisegc/gc_thread.hpp
        include/libprecisegc/details/gc_tagging.hpp
        include/libprecisegc/details/threads/stack_bitmap.hpp
        include/libprecisegc/details/threads/return_address.hpp
        include/libprecisegc/details/threads/static_root_set.hpp include/libprecisegc/details/compacting/fix_ptrs.hpp include/libprecisegc/details/compacting/forwarding.hpp include/libprecisegc/details/compacting/two_finger_compact.hpp)

set(precisegc_LIB_SRCS
        src/details/gc_mark.cpp
        src/details/gc_heap.cpp
        src/details/threads/gc_new_stack.cpp
        src/details/ptrs/gc_untyped_ptr.cpp
        src/details/logging.cpp
        src/details/allocators/bitmap_pool_chunk.cpp
        src/details/allocators/managed_pool_chunk.cpp
        src/details/managed_ptr.cpp
        src/details/forwarding.cpp
        src/details/threads/pending_call.cpp
        src/details/threads/posix_signal.cpp
        src/details/threads/ass_sync.cpp
        src/details/threads/posix_stw_manager.cpp
        src/details/threads/thread_manager.cpp
        src/details/garbage_collector.cpp
        src/details/collectors/initiation_policy.cpp
        src/details/serial_gc.cpp
        src/details/collectors/incremental_gc.cpp
        src/details/allocators/freelist_pool_chunk.cpp
        src/details/gc_hooks.cpp
        src/details/gc_factory.cpp
        src/gc.cpp src/details/collectors/packet_manager.cpp
        src/details/threads/this_managed_thread.cpp
        src/details/threads/managed_thread_accessor.cpp
        src/details/allocators/managed_object_descriptor.cpp
        src/details/gc_manager.cpp src/details/allocators/stack_chunk.cpp
        src/details/allocators/core_allocator.cpp
        src/details/threads/stack_bitmap.cpp
        src/details/threads/managed_thread.cpp
        src/details/threads/static_root_set.cpp src/details/compacting/forwarding.cpp)


find_package(Threads REQUIRED)
find_package(Boost 1.54)

include_directories(${Boost_INCLUDE_DIR})
include_directories(include)
include_directories(include/libprecisegc)
include_directories(include/libprecisegc/details)

add_library(libprecisegc STATIC ${precisegc_LIB_HDRS} ${precisegc_LIB_SRCS})

target_link_libraries(libprecisegc "${CMAKE_THREAD_LIBS_INIT}")

set_target_properties(libprecisegc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS libprecisegc ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
foreach ( file ${precisegc_LIB_HDRS} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES ${file} DESTINATION ${CMAKE_INSTALL_PREFIX}/${dir} )
endforeach()
