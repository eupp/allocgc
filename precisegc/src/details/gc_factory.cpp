#include <libprecisegc/details/gc_factory.hpp>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/collectors/serial_gc.hpp>
#include <libprecisegc/details/collectors/incremental_gc.hpp>
#include <libprecisegc/details/initiation_policy.hpp>

namespace precisegc { namespace details {

using namespace collectors;

std::unique_ptr<initiation_policy> gc_factory::create_space_based_policy(gc_strategy* gc, size_t max_heap_size)
{
    static const size_t start_heap_size     = 2 * 1024 * 1024;  // 2 Mb
    static const double increase_factor     = 2.0;
    static const double marking_threshold   = 0.6;
    static const double sweeping_threshold  = 1.0;

    return utils::make_unique<space_based_policy>(
              gc
            , max_heap_size == std::numeric_limits<size_t>::max() ? start_heap_size : max_heap_size
            , marking_threshold
            , sweeping_threshold
            , increase_factor
            , max_heap_size
    );
}

std::unique_ptr<initiation_policy> gc_factory::create_initiation_policy(gc_strategy* gc,
                                                                        const gc_options& options)
{
    if (options.init == gc_init_strategy::MANUAL) {
        return utils::make_unique<empty_policy>();
    } else if (options.init == gc_init_strategy::SPACE_BASED || options.init == gc_init_strategy::DEFAULT) {
        return create_space_based_policy(gc, options.heapsize);
    }
}

std::unique_ptr<gc_strategy> gc_factory::create_gc(const gc_options& options)
{
    if (options.type == gc_type::SERIAL) {
        if (options.compacting == gc_compacting::DISABLED) {
            return utils::make_unique<serial_gc>(options.threads_available);
        } else if (options.compacting == gc_compacting::ENABLED) {
            return utils::make_unique<serial_compacting_gc>(options.threads_available);
        }
    } else if (options.type == gc_type::INCREMENTAL) {
        if (options.compacting == gc_compacting::DISABLED) {
            return utils::make_unique<incremental_gc>(options.threads_available);
        } else if (options.compacting == gc_compacting::ENABLED) {
            return utils::make_unique<incremental_compacting_gc>(options.threads_available);
        }
    }
}

}}
