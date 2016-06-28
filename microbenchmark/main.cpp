#include <nonius/main.h++>

#include <libprecisegc/libprecisegc.hpp>

using namespace precisegc;

int main(int argc, char** argv) {
    gc_options options = {
              .type         = gc_type::SERIAL
            , .compacting   = gc_compacting::DISABLED
            , .loglevel     = gc_loglevel::DEBUG
            , .print_stat   = false
    };
    gc_init(options);
    return nonius::main(argc, argv);
}