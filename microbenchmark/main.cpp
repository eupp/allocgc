#include <nonius/main.h++>

#include <libprecisegc/libprecisegc.hpp>

using namespace precisegc;

int main(int argc, char** argv) {
    gc_init_options options;
    options.algo = gc_algo::INCREMENTAL;
    options.initiation = gc_initiation::MANUAL;
    gc_init(options);
    return nonius::main(argc, argv);
}