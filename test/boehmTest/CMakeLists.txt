set(TEST boehm)

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
file(GLOB SOURCE "*.cpp" "../../precisegc/sources/*.cpp")
add_executable(${TEST} EXCLUDE_FROM_ALL ${SOURCE})

#find_path(PRECISEGC_PATH libprecisegc/libprecisegc.h)
#include_directories(${PRECISEGC_PATH})

include_directories(${PROJECT_SOURCE_DIR}/precisegc/include/)

find_package(Threads REQUIRED)

#set(LIBS ${PRECISEGC_LIB} ${PTHREAD_LIB})

target_link_libraries(${TEST} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${TEST} libprecisegc)