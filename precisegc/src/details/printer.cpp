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
            "   GC STAT SUMMARY                                                          \n"
            "pause time xxxx yy                                                          \n"
            "shrunk     xxxx yy                                                          \n"
            "swept      xxxx yy                                                          \n"
            "****************************************************************************\n";

    static const std::string placeholder = "xxxx yy";

    static size_t pause_time_pos = text.find(placeholder, 0);
    static size_t shrunk_pos     = text.find(placeholder, pause_time_pos + placeholder.size());
    static size_t swept_pos      = text.find(placeholder, shrunk_pos + placeholder.size());

    text.replace(pause_time_pos, placeholder.size(), duration_str(pause_stat.duration));
    text.replace(shrunk_pos, placeholder.size(), size_str(sweep_stat.shrunk));
    text.replace(swept_pos, placeholder.size(), size_str(sweep_stat.swept));

    m_stream << text;
}

void printer::print_pause_stat(const gc_pause_stat& pause_stat)
{

}

std::string printer::str(size_t n)
{
    static char buf[5] = {0};
    snprintf(buf, 4, "%4u", n);
    return std::string(buf);
}

std::string printer::size_str(size_t size)
{
    static const size_t Kb = 1024;
    static const size_t Mb = 1024 * Kb;
    static const size_t Gb = 1024 * Mb;

    if (size >= Gb) {
        return str(size / Gb) + " Gb";
    } else if (size >= Mb) {
        return str(size / Mb) + " Mb";
    } else if (size >= Kb) {
        return str(size / Kb) + " Kb";
    } else {
        return str(size) + " b";
    }
}

std::string printer::duration_str(gc_duration duration)
{
    static const size_t ms = 1000;
    static const size_t s = 1000 * ms;

    auto dur_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    if (dur_us >= s) {
        return str(dur_us / s) + " s";
    } else if (dur_us >= ms) {
        return str(dur_us / ms) + " ms";
    } else {
        return str(dur_us) + " us";
    }
}

const char* printer::pause_type_str(gc_pause_type pause_type)
{
    return "";
}

}}
