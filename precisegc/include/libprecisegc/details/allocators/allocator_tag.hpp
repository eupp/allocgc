#ifndef DIPLOMA_ALLOCATOR_TAG_HPP
#define DIPLOMA_ALLOCATOR_TAG_HPP

#include <type_traits>

namespace precisegc { namespace details { namespace allocators {

class stateful_alloc_tag {};
class stateless_alloc_tag {};

class multi_block_chunk_tag {};
class single_block_chunk_tag {};

template <typename Chunk>
using is_multi_block_chunk = std::is_same<typename Chunk::chunk_tag, multi_block_chunk_tag>;

}}}

#endif //DIPLOMA_ALLOCATOR_TAG_HPP
