#ifndef DIPLOMA_ALLOCATORS_TYPES_H
#define DIPLOMA_ALLOCATORS_TYPES_H

#include <cstdint>
#include <atomic>

namespace precisegc { namespace details {

typedef std::uint8_t byte;
typedef std::atomic<byte*> atomic_byte_ptr;

}}

#endif //DIPLOMA_ALLOCATORS_TYPES_H