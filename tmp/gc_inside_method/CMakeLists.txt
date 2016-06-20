set(TEST gc_inside_method)

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
file(GLOB SOURCE "*.cpp")
add_executable(${TEST} EXCLUDE_FROM_ALL ${SOURCE})


find_library(PTHREAD_LIB pthread REQUIRED)

set(LIBS ${PTHREAD_LIB})
target_link_libraries(${TEST} ${LIBS})