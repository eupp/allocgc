#include <libprecisegc/details/printer.hpp>

#include <cstdio>

namespace precisegc { namespace details {

printer::printer(const std::ostream& stream)
    : m_stream(stream.rdbuf())
{
    m_stream.rdbuf()->pubsetbuf(0, 0);
}

void printer::print_sweep_stat(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat)
{
    static std::string text =
            "****************************************************************************\n"
            "   GC SWEEP SUMMARY                                                         \n"
            "pause type: xxxx yy                                                          \n"
            "pause time: xxxx yy                                                          \n"
            "shrunk:     xxxx yy                                                          \n"
            "swept:      xxxx yy                                                          \n"
            "****************************************************************************\n";

    static const std::string placeholder = "xxxx yy";

    static size_t pause_type_pos = text.find(placeholder, 0);
    static size_t pause_time_pos = text.find(placeholder, pause_type_pos + placeholder.size());
    static size_t shrunk_pos     = text.find(placeholder, pause_time_pos + placeholder.size());
    static size_t swept_pos      = text.find(placeholder, shrunk_pos + placeholder.size());

    std::string pause_type = pause_type_to_str(pause_stat.type);

    text.replace(pause_type_pos, pause_type.size(), pause_type);
    text.replace(pause_time_pos, placeholder.size(), duration_to_str(pause_stat.duration));
    text.replace(shrunk_pos, placeholder.size(), heapsize_to_str(sweep_stat.shrunk));
    text.replace(swept_pos, placeholder.size(), heapsize_to_str(sweep_stat.swept));

    m_stream << text;
}

void printer::print_pause_stat(const gc_pause_stat& pause_stat)
{
    static std::string text =
            "****************************************************************************\n"
            "   GC PAUSE SUMMARY                                                         \n"
            "pause type: xxxx yy                                                          \n"
            "pause time: xxxx yy                                                          \n"
            "****************************************************************************\n";

    static const std::string placeholder = "xxxx yy";

    static size_t pause_type_pos = text.find(placeholder, 0);
    static size_t pause_time_pos = text.find(placeholder, pause_type_pos + placeholder.size());

    std::string pause_type = pause_type_to_str(pause_stat.type);

    text.replace(pause_type_pos, pause_type.size(), pause_type);
    text.replace(pause_time_pos, placeholder.size(), duration_to_str(pause_stat.duration));

    m_stream << text;
}

}}
