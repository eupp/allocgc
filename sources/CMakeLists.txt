set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -O2")

set(precisegc_LIB_SRCS
	debug_print.h
    deref_roots.h deref_roots.cpp
    fake_roots.h
    gc_malloc.h gc_malloc.cpp
    gc_new.h
    gc_ptr.h
    go.h go.cpp
    index_tree.h index_tree.cpp
    malloc.h malloc.cpp
    mark.cpp
    meta_information.h
    stack.h stack.cpp
    taginfo.h taginfo.cpp
    threading.h threading.cpp
    tlvars.h tlvars.cpp)

add_library(precisegc SHARED ${precisegc_LIB_SRCS})

find_library(PTHREAD_LIB pthread REQUIRED)
find_path(PTHREAD_PATH pthread.h)
include_directories(${PTHREAD_PATH})
target_link_libraries(precisegc ${PTHREAD_LIB})

set_target_properties(precisegc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS precisegc
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES ${precisegc_LIB_SRCS} DESTINATION include/libprecisegc)