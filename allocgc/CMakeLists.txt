
set(precisegc_LIB_HDRS
        include/liballocgc/gc_ptr.hpp
        include/liballocgc/gc_new.hpp
        include/liballocgc/gc.hpp
        include/liballocgc/liballocgc.hpp

        include/liballocgc/gc_type_meta.hpp
        include/liballocgc/details/gc_heap.hpp
        include/liballocgc/details/constants.hpp
        include/liballocgc/details/utils/math.hpp
        include/liballocgc/details/utils/barrier.hpp
        include/liballocgc/details/utils/utility.hpp
        include/liballocgc/details/collectors/gc_new_stack_entry.hpp
        include/liballocgc/details/gc_unsafe_scope.hpp
        include/liballocgc/details/ptrs/gc_untyped_ptr.hpp
        include/liballocgc/details/logging.hpp
        include/liballocgc/details/allocators/gc_core_allocator.hpp
        include/liballocgc/details/allocators/index_tree.hpp
        include/liballocgc/details/allocators/debug_layer.hpp
        include/liballocgc/details/allocators/bucket_allocator.hpp
        include/liballocgc/details/allocators/bitmap_pool_chunk.hpp
        include/liballocgc/details/allocators/stl_adapter.hpp
        include/liballocgc/details/allocators/pow2_bucket_policy.hpp
        include/liballocgc/details/allocators/gc_pool_descriptor.hpp
        include/liballocgc/details/allocators/gc_memory_descriptor.hpp
        include/liballocgc/details/utils/flatten_range.hpp
        include/liballocgc/details/threads/pending_call.hpp
        include/liballocgc/details/threads/posix_signal.hpp
        include/liballocgc/details/collectors/pin_set.hpp
        include/liballocgc/details/threads/ass_sync.hpp
        include/liballocgc/gc_exception.hpp
        include/liballocgc/details/threads/gc_thread_manager.hpp
        include/liballocgc/details/threads/stw_manager.hpp
        include/liballocgc/details/utils/scoped_thread.hpp
        include/liballocgc/details/utils/scope_guard.hpp
        include/liballocgc/details/threads/posix_thread.hpp
        include/liballocgc/details/gc_facade.hpp
        include/liballocgc/details/collectors/marker.hpp
        include/liballocgc/details/utils/locked_range.hpp
        include/liballocgc/details/utils/system_error.hpp
        include/liballocgc/gc_strategy.hpp
        include/liballocgc/details/utils/to_string.hpp
        include/liballocgc/details/utils/make_reverse_iterator.hpp
        include/liballocgc/details/allocators/allocator_tag.hpp
        include/liballocgc/details/utils/bitset.hpp
        include/liballocgc/details/collectors/gc_serial.hpp
        include/liballocgc/details/threads/world_snapshot.hpp
        include/liballocgc/details/utils/make_unique.hpp
        include/liballocgc/details/collectors/gc_incremental.hpp
        include/liballocgc/details/gc_clock.hpp
        include/liballocgc/details/gc_interface.hpp
        include/liballocgc/details/allocators/default_allocator.hpp
        include/liballocgc/details/utils/dynarray.hpp
        include/liballocgc/details/allocators/freelist_pool_chunk.hpp
        include/liballocgc/gc_pin.hpp
        include/liballocgc/gc_ref.hpp
        include/liballocgc/gc_handle.hpp
        include/liballocgc/details/gc_hooks.hpp
        include/liballocgc/details/utils/base_offset.hpp
        include/liballocgc/details/utils/static_thread_pool.hpp
        include/liballocgc/details/utils/dummy_mutex.hpp
        include/liballocgc/details/collectors/packet_manager.hpp
        include/liballocgc/details/utils/safepoint_lock.hpp
        include/liballocgc/details/allocators/gc_object_descriptor.hpp
        include/liballocgc/details/gc_manager.hpp
        include/liballocgc/details/utils/block_ptr.hpp
        include/liballocgc/details/allocators/sys_allocator.hpp
        include/liballocgc/details/allocators/freelist_allocator.hpp
        include/liballocgc/details/utils/get_ptr.hpp
        include/liballocgc/details/allocators/pool.hpp
        include/liballocgc/details/allocators/sys_allocator.hpp
        include/liballocgc/details/allocators/null_allocator.hpp
        include/liballocgc/details/collectors/pin_stack.hpp
        include/liballocgc/gc_thread.hpp
        include/liballocgc/details/collectors/stack_bitmap.hpp
        include/liballocgc/details/threads/return_address.hpp
        include/liballocgc/details/collectors/static_root_set.hpp
        include/liballocgc/details/compacting/fix_ptrs.hpp
        include/liballocgc/details/compacting/forwarding.hpp
        include/liballocgc/details/compacting/two_finger_compactor.hpp
        include/liballocgc/details/collectors/remset.hpp
        include/liballocgc/details/allocators/memory_index.hpp
        include/liballocgc/details/allocators/gc_so_allocator.hpp
        include/liballocgc/details/allocators/gc_lo_allocator.hpp
        include/liballocgc/details/allocators/gc_pool_allocator.hpp
        include/liballocgc/gc_alloc.hpp
        include/liballocgc/details/allocators/list_allocator.hpp
        include/liballocgc/details/allocators/pool_allocator.hpp
        include/liballocgc/details/gc_cell.hpp
        include/liballocgc/details/allocators/gc_box.hpp
        include/liballocgc/gc_cast.hpp
        include/liballocgc/gc_common.hpp
        include/liballocgc/details/collectors/gc_core.hpp
        include/liballocgc/details/threads/gc_thread_descriptor.hpp
        include/liballocgc/details/collectors/gc_new_stack.hpp
        include/liballocgc/details/allocators/memory_descriptor.hpp
        include/liballocgc/details/collectors/conservative_stack.hpp
        include/liballocgc/details/collectors/conservative_pin_set.hpp include/liballocgc/gc_factory.hpp)

set(precisegc_LIB_SRCS
        src/details/gc_heap.cpp
        src/details/ptrs/gc_untyped_ptr.cpp
        src/details/logging.cpp
        src/details/allocators/bitmap_pool_chunk.cpp
        src/details/allocators/gc_pool_descriptor.cpp
        src/details/threads/pending_call.cpp
        src/details/threads/posix_signal.cpp
        src/details/threads/ass_sync.cpp
        src/details/threads/posix_stw_manager.cpp
        src/details/threads/gc_thread_manager.cpp
        src/details/gc_facade.cpp
        src/details/gc_serial.cpp
        src/details/collectors/gc_incremental.cpp
        src/details/allocators/freelist_pool_chunk.cpp
        src/details/gc_hooks.cpp
        src/gc.cpp src/details/collectors/packet_manager.cpp
        src/details/allocators/gc_object_descriptor.cpp
        src/details/gc_manager.cpp
        src/details/allocators/gc_core_allocator.cpp
        src/details/threads/stack_bitmap.cpp
        src/details/threads/static_root_set.cpp
        src/details/compacting/forwarding.cpp
        src/details/collectors/remset.cpp
        src/details/collectors/memory_index.cpp
        src/details/allocators/gc_so_allocator.cpp
        src/details/allocators/gc_pool_allocator.cpp
        src/details/allocators/gc_lo_allocator.cpp
        src/details/collectors/gc_core.cpp
        src/details/collectors/marker.cpp)


find_package(Threads REQUIRED)
find_package(Boost 1.54)

include_directories(${Boost_INCLUDE_DIR})
include_directories(include)
include_directories(include/liballocgc)
include_directories(include/liballocgc/details)

add_library(liballocgc STATIC ${precisegc_LIB_HDRS} ${precisegc_LIB_SRCS})

target_link_libraries(liballocgc "${CMAKE_THREAD_LIBS_INIT}")

set_target_properties(liballocgc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS liballocgc ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
foreach ( file ${precisegc_LIB_HDRS} )
    get_filename_component( dir ${file} DIRECTORY )
    install( FILES ${file} DESTINATION ${CMAKE_INSTALL_PREFIX}/${dir} )
endforeach()
