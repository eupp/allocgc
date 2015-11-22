
set(precisegc_LIB_PUB_HDRS
        include/libprecisegc/debug_print.h
        include/libprecisegc/fake_roots.h
        include/libprecisegc/gc_malloc.h
        include/libprecisegc/gc_ptr.h
        include/libprecisegc/go.h
        include/libprecisegc/gc_new.h
        include/libprecisegc/heap.h
        include/libprecisegc/index_tree.h
        include/libprecisegc/stack.h
        include/libprecisegc/taginfo.h
        include/libprecisegc/tlvars.h
        include/libprecisegc/threading.h
        include/libprecisegc/meta_information.h
        include/libprecisegc/deref_roots.h

        include/libprecisegc/details/class_meta.h
        include/libprecisegc/details/mutex.h
        include/libprecisegc/details/os.h
        include/libprecisegc/details/segregated_storage.h include/libprecisegc/details/constants.h include/libprecisegc/details/os_mem_ptr.h)


set(precisegc_LIB_SRCS
        src/deref_roots.cpp
        src/gc_malloc.cpp
        src/go.cpp
        src/index_tree.cpp
        src/heap.cpp
        src/mark.cpp
        src/stack.cpp
        src/taginfo.cpp
        src/threading.cpp
        src/tlvars.cpp
        src/page_descriptor.cpp src/segregated_storage.cpp)


include_directories(include/libprecisegc)
include_directories(include/libprecisegc/details)

add_library(libprecisegc SHARED ${precisegc_LIB_PUB_HDRS} ${precisegc_LIB_PRIV_HDRS} ${precisegc_LIB_SRCS})

find_package(Threads REQUIRED)
target_link_libraries(libprecisegc "${CMAKE_THREAD_LIBS_INIT}")

set_target_properties(libprecisegc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS libprecisegc LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${precisegc_LIB_PUB_HDRS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libprecisegc)