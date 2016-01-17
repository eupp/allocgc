
set(precisegc_LIB_HDRS
        include/libprecisegc/debug_print.h
        include/libprecisegc/fake_roots.h
        include/libprecisegc/gc_ptr.h
        include/libprecisegc/go.h
        include/libprecisegc/gc_new.h
        include/libprecisegc/gcmalloc.h
        include/libprecisegc/index_tree.h
        include/libprecisegc/stack.h
        include/libprecisegc/taginfo.h
        include/libprecisegc/tlvars.h
        include/libprecisegc/threading.h
        include/libprecisegc/meta_information.h
        include/libprecisegc/deref_roots.h
        include/libprecisegc/object.h

        include/libprecisegc/details/class_meta.h
        include/libprecisegc/details/mutex.h
        include/libprecisegc/details/os.h
        include/libprecisegc/details/page_descriptor.h
        include/libprecisegc/details/segregated_list.h
        include/libprecisegc/details/heap.h
        include/libprecisegc/details/constants.h
        include/libprecisegc/details/os_mem_ptr.h
        include/libprecisegc/details/util.h
        include/libprecisegc/details/forwarding_list.h
        include/libprecisegc/gc_pin.h
        include/libprecisegc/details/iterator_base.h
        include/libprecisegc/details/iterator_access.h
        include/libprecisegc/details/pool_chunk.h
        include/libprecisegc/details/condition_variable.h
        include/libprecisegc/details/gc_pause.h
        include/libprecisegc/details/barrier.h)


set(precisegc_LIB_SRCS
        src/deref_roots.cpp
        src/go.cpp
        src/index_tree.cpp
        src/gcmalloc.cpp
        src/mark.cpp
        src/stack.cpp
        src/taginfo.cpp
        src/threading.cpp
        src/tlvars.cpp
        src/page_descriptor.cpp
        src/segregated_list.cpp
        src/heap.cpp src/pool_chunk.cpp)


include_directories(include/libprecisegc)
include_directories(include/libprecisegc/details)

add_library(libprecisegc SHARED ${precisegc_LIB_HDRS} ${precisegc_LIB_SRCS})

find_package(Threads REQUIRED)
target_link_libraries(libprecisegc "${CMAKE_THREAD_LIBS_INIT}")

set_target_properties(libprecisegc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS libprecisegc LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${precisegc_LIB_HDRS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libprecisegc)