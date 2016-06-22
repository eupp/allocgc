
set(precisegc_LIB_HDRS
        include/libprecisegc/gc_ptr.h
        include/libprecisegc/gc_new.h
        include/libprecisegc/gc.h
        include/libprecisegc/gc_init.h
        include/libprecisegc/gc_options.hpp
        include/libprecisegc/libprecisegc.h

        include/libprecisegc/details/type_meta.hpp
        include/libprecisegc/details/object_meta.h
        include/libprecisegc/details/gc_heap.h
        include/libprecisegc/details/constants.hpp
        include/libprecisegc/details/utils/math.h
        include/libprecisegc/details/forwarding.h
        include/libprecisegc/details/utils/barrier.hpp
        include/libprecisegc/details/utils/utility.hpp
        include/libprecisegc/details/gc_compact.h
        include/libprecisegc/details/ptrs/gc_new_stack.hpp
        include/libprecisegc/details/ptrs/gc_new_impl.hpp
        include/libprecisegc/details/gc_unsafe_scope.h
        include/libprecisegc/details/ptrs/gc_untyped_ptr.hpp
        include/libprecisegc/details/ptrs/gc_untyped_pin.hpp
        include/libprecisegc/details/ptrs/gc_ptr_access.hpp
        include/libprecisegc/details/logging.h
        include/libprecisegc/details/barrier_buffer.h
        include/libprecisegc/details/gc_mark.h
        include/libprecisegc/details/initator.hpp
        include/libprecisegc/details/allocators/page_allocator.hpp
        include/libprecisegc/details/index_tree.h
        include/libprecisegc/details/types.hpp
        include/libprecisegc/details/allocators/debug_layer.h
        include/libprecisegc/details/allocators/bucket_allocator.hpp
        include/libprecisegc/details/allocators/bitmap_pool_chunk.h
        include/libprecisegc/details/allocators/list_allocator.hpp
        include/libprecisegc/details/allocators/stl_adapter.hpp
        include/libprecisegc/details/allocators/pow2_bucket_policy.h
        include/libprecisegc/details/managed_pool_chunk.hpp
        include/libprecisegc/details/memory_descriptor.hpp
        include/libprecisegc/details/managed_ptr.hpp
        include/libprecisegc/details/utils/flatten_range.hpp
        include/libprecisegc/details/threads/pending_call.hpp
        include/libprecisegc/details/threads/posix_signal.hpp
        include/libprecisegc/details/threads/managed_thread.hpp
        include/libprecisegc/details/stack_map.hpp
        include/libprecisegc/details/threads/ass_sync.hpp
        include/libprecisegc/details/gc_exception.hpp
        include/libprecisegc/details/threads/thread_manager.hpp
        include/libprecisegc/details/threads/stw_manager.hpp
        include/libprecisegc/details/utils/scoped_thread.hpp
        include/libprecisegc/details/utils/scope_guard.hpp
        include/libprecisegc/details/threads/posix_thread.hpp
        include/libprecisegc/details/garbage_collector.hpp
        include/libprecisegc/details/marker.hpp
        include/libprecisegc/details/utils/lock_range.hpp
        include/libprecisegc/details/ptrs/trace_ptr.hpp
        include/libprecisegc/details/utils/system_error.hpp
        include/libprecisegc/details/gc_strategy.hpp
        include/libprecisegc/details/utils/to_string.hpp
        include/libprecisegc/details/utils/make_reverse_iterator.hpp
        include/libprecisegc/details/allocators/allocator_tag.hpp
        include/libprecisegc/details/utils/bitset.hpp
        include/libprecisegc/details/initation_policy.hpp
        include/libprecisegc/details/serial_gc.hpp
        include/libprecisegc/details/threads/world_snapshot.hpp
        include/libprecisegc/details/utils/make_unique.hpp
        include/libprecisegc/details/incremental_gc.hpp
        include/libprecisegc/details/recorder.hpp
        include/libprecisegc/details/printer.hpp
        include/libprecisegc/details/gc_clock.hpp
        include/libprecisegc/details/gc_interface.hpp
        include/libprecisegc/details/allocators/default_allocator.hpp
        include/libprecisegc/details/utils/dynarray.hpp
        include/libprecisegc/details/allocators/freelist_pool_chunk.hpp
        include/libprecisegc/details/allocators/intrusive_list_allocator.hpp include/libprecisegc/gc_pin.hpp include/libprecisegc/gc_ref.hpp include/libprecisegc/details/gc_handle.hpp)

set(precisegc_LIB_SRCS
        src/gc_mark.cpp
        src/gc_heap.cpp
        src/details/ptrs/gc_new_stack.cpp
        src/details/ptrs/gc_untyped_ptr.cpp
        src/details/ptrs/gc_untyped_pin.cpp
        src/logging.cpp
        src/gc_init.cpp
        src/barrier_buffer.cpp
        src/details/initator.cpp
        src/details/allocators/bitmap_pool_chunk.cpp
        src/details/managed_pool_chunk.cpp
        src/details/managed_ptr.cpp
        src/forwarding.cpp
        src/details/threads/pending_call.cpp
        src/details/threads/posix_signal.cpp
        src/details/threads/ass_sync.cpp
        src/details/threads/posix_stw_manager.cpp
        src/details/threads/thread_manager.cpp
        src/details/marker.cpp src/details/garbage_collector.cpp src/details/initation_policy.cpp
        src/details/serial_gc.cpp
        src/details/incremental_gc.cpp
        src/details/printer.cpp
        src/details/recorder.cpp
        src/details/allocators/freelist_pool_chunk.cpp src/details/gc_handle.cpp)


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
install(FILES ${precisegc_LIB_HDRS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libprecisegc)