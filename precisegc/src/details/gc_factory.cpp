#include <libprecisegc/details/gc_factory.hpp>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/collectors/gc_serial.hpp>
#include <libprecisegc/details/collectors/gc_incremental.hpp>

namespace precisegc { namespace details {

using namespace collectors;

std::unique_ptr<gc_strategy> gc_factory::create_gc(const gc_init_options& options)
{
    if (options.algo == gc_algo::SERIAL) {
        if (options.compacting == gc_compacting::DISABLED) {
            return utils::make_unique<gc_serial>(options.threads_available);
        } else if (options.compacting == gc_compacting::ENABLED) {
            return utils::make_unique<gc_serial>(options.threads_available);
        }
    } else if (options.algo == gc_algo::INCREMENTAL) {
        if (options.compacting == gc_compacting::DISABLED) {
            return utils::make_unique<gc_incremental>(options.threads_available);
        } else if (options.compacting == gc_compacting::ENABLED) {
            return utils::make_unique<gc_incremental>(options.threads_available);
        }
    }
}

}}
