#ifndef DIPLOMA_PRINTER_HPP
#define DIPLOMA_PRINTER_HPP

#include <string>
#include <iostream>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class printer : private utils::noncopyable, private utils::nonmovable
{
public:
    printer(const std::ostream& stream);

    void print_sweep_stat(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat);
    void print_pause_stat(const gc_pause_stat& pause_stat);
private:
    static std::string str(size_t n);
    static std::string size_str(size_t size);
    static std::string duration_str(gc_duration duration);
    static const char* pause_type_str(gc_pause_type pause_type);

    std::ostream m_stream;
};

}}

#endif //DIPLOMA_PRINTER_HPP
