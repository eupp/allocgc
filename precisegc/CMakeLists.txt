
set(precisegc_LIB_HDRS
        include/libprecisegc/gc_ptr.hpp
        include/libprecisegc/gc_new.hpp
        include/libprecisegc/gc.hpp
        include/libprecisegc/libprecisegc.hpp

        include/libprecisegc/gc_type_meta.hpp
        include/libprecisegc/details/gc_heap.hpp
        include/libprecisegc/details/constants.hpp
        include/libprecisegc/details/utils/math.hpp
        include/libprecisegc/details/utils/barrier.hpp
        include/libprecisegc/details/utils/utility.hpp
        include/libprecisegc/details/collectors/gc_new_stack_entry.hpp
        include/libprecisegc/details/gc_unsafe_scope.hpp
        include/libprecisegc/details/ptrs/gc_untyped_ptr.hpp
        include/libprecisegc/details/logging.hpp
        include/libprecisegc/details/allocators/gc_core_allocator.hpp
        include/libprecisegc/details/allocators/index_tree.hpp
        include/libprecisegc/details/allocators/debug_layer.hpp
        include/libprecisegc/details/allocators/bucket_allocator.hpp
        include/libprecisegc/details/allocators/bitmap_pool_chunk.hpp
        include/libprecisegc/details/allocators/stl_adapter.hpp
        include/libprecisegc/details/allocators/pow2_bucket_policy.hpp
        include/libprecisegc/details/allocators/gc_pool_descriptor.hpp
        include/libprecisegc/details/allocators/gc_memory_descriptor.hpp
        include/libprecisegc/details/utils/flatten_range.hpp
        include/libprecisegc/details/threads/pending_call.hpp
        include/libprecisegc/details/threads/posix_signal.hpp
        include/libprecisegc/details/collectors/pin_set.hpp
        include/libprecisegc/details/threads/ass_sync.hpp
        include/libprecisegc/gc_exception.hpp
        include/libprecisegc/details/threads/gc_thread_manager.hpp
        include/libprecisegc/details/threads/stw_manager.hpp
        include/libprecisegc/details/utils/scoped_thread.hpp
        include/libprecisegc/details/utils/scope_guard.hpp
        include/libprecisegc/details/threads/posix_thread.hpp
        include/libprecisegc/details/gc_facade.hpp
        include/libprecisegc/details/collectors/marker.hpp
        include/libprecisegc/details/utils/locked_range.hpp
        include/libprecisegc/details/utils/system_error.hpp
        include/libprecisegc/gc_strategy.hpp
        include/libprecisegc/details/utils/to_string.hpp
        include/libprecisegc/details/utils/make_reverse_iterator.hpp
        include/libprecisegc/details/allocators/allocator_tag.hpp
        include/libprecisegc/details/utils/bitset.hpp
        include/libprecisegc/details/collectors/gc_serial.hpp
        include/libprecisegc/details/threads/world_snapshot.hpp
        include/libprecisegc/details/utils/make_unique.hpp
        include/libprecisegc/details/collectors/gc_incremental.hpp
        include/libprecisegc/details/gc_clock.hpp
        include/libprecisegc/details/gc_interface.hpp
        include/libprecisegc/details/allocators/default_allocator.hpp
        include/libprecisegc/details/utils/dynarray.hpp
        include/libprecisegc/details/allocators/freelist_pool_chunk.hpp
        include/libprecisegc/gc_pin.hpp
        include/libprecisegc/gc_ref.hpp
        include/libprecisegc/gc_handle.hpp
        include/libprecisegc/details/gc_hooks.hpp
        include/libprecisegc/details/utils/base_offset.hpp
        include/libprecisegc/details/utils/static_thread_pool.hpp
        include/libprecisegc/details/utils/dummy_mutex.hpp
        include/libprecisegc/details/collectors/packet_manager.hpp
        include/libprecisegc/details/utils/safepoint_lock.hpp
        include/libprecisegc/details/allocators/gc_object_descriptor.hpp
        include/libprecisegc/details/gc_manager.hpp
        include/libprecisegc/details/utils/block_ptr.hpp
        include/libprecisegc/details/allocators/sys_allocator.hpp
        include/libprecisegc/details/allocators/freelist_allocator.hpp
        include/libprecisegc/details/utils/get_ptr.hpp
        include/libprecisegc/details/allocators/pool.hpp
        include/libprecisegc/details/allocators/sys_allocator.hpp
        include/libprecisegc/details/allocators/null_allocator.hpp
        include/libprecisegc/details/collectors/pin_stack.hpp
        include/libprecisegc/gc_thread.hpp
        include/libprecisegc/details/collectors/stack_bitmap.hpp
        include/libprecisegc/details/threads/return_address.hpp
        include/libprecisegc/details/collectors/static_root_set.hpp
        include/libprecisegc/details/compacting/fix_ptrs.hpp
        include/libprecisegc/details/compacting/forwarding.hpp
        include/libprecisegc/details/compacting/two_finger_compactor.hpp
        include/libprecisegc/details/collectors/remset.hpp
        include/libprecisegc/details/allocators/memory_index.hpp
        include/libprecisegc/details/allocators/gc_so_allocator.hpp
        include/libprecisegc/gc_type_meta_factory.hpp
        include/libprecisegc/details/allocators/gc_lo_allocator.hpp
        include/libprecisegc/details/allocators/gc_pool_allocator.hpp
        include/libprecisegc/gc_alloc.hpp
        include/libprecisegc/details/allocators/list_allocator.hpp
        include/libprecisegc/details/allocators/pool_allocator.hpp
        include/libprecisegc/details/gc_cell.hpp
        include/libprecisegc/details/allocators/gc_box.hpp
        include/libprecisegc/gc_cast.hpp
        include/libprecisegc/gc_common.hpp
        include/libprecisegc/details/collectors/gc_core.hpp
        include/libprecisegc/details/threads/gc_thread_descriptor.hpp
        include/libprecisegc/details/collectors/gc_new_stack.hpp
        include/libprecisegc/details/allocators/memory_descriptor.hpp
        include/libprecisegc/details/collectors/conservative_stack.hpp
        include/libprecisegc/details/collectors/conservative_pin_set.hpp include/libprecisegc/gc_factory.hpp)

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
