#include <libprecisegc/details/allocators/gc_cell.hpp>

namespace precisegc { namespace details { namespace allocators {

byte* gc_cell::get() const
{
    return m_cell;
}

memory_descriptor* descriptor() const
{

}

bool get_mark() const;
bool get_pin() const;

void set_mark(bool mark);
void set_pin(bool pin);

gc_lifetime_tag get_lifetime_tag() const;

size_t cell_size() const;
byte*  cell_start() const;

size_t object_count() const;
const gc_type_meta* get_type_meta() const;

void mark_initilized(const gc_type_meta* tmeta = nullptr);

void move_from(byte* from, memory_descriptor* from_descr);
void finalize();

}}}