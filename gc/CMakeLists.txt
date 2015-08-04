set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -O2")

set(precisegc_LIB_SRCS
    gc_new.h
    gc_ptr.h
    go.cpp
    go.h
    libprecisegc.h
    tlvars.h
    meta_information.h
    stack.cpp
    stack.h
    debug_print.h
    deref_roots.cpp
    threading.cpp
    malloc.cpp
    tlvars.cpp)

add_library(precisegc SHARED ${precisegc_LIB_SRCS})

find_library(PTHREAD_LIB pthread REQUIRED)
find_path(PTHREAD_PATH pthread.h)
include_directories(${PTHREAD_PATH})
target_link_libraries(precisegc ${PTHREAD_LIB})

set_target_properties(precisegc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS precisegc
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES libprecisegc.h tlvars.h gc_new.h gc_ptr.h go.h meta_information.h stack.h debug_print.h deref_roots.h threading.h malloc.h DESTINATION include/libprecisegc)