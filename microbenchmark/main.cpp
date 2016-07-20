#include <nonius/main.h++>

#include <libprecisegc/libprecisegc.hpp>

using namespace precisegc;

int main(int argc, char** argv) {
    gc_options options;
    options.init = gc_init_strategy::MANUAL;
    gc_init(options);
    return nonius::main(argc, argv);
}