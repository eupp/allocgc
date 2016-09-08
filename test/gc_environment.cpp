#include <gtest/gtest.h>

#include <cassert>

#include "libprecisegc/libprecisegc.hpp"

using namespace precisegc;

class gc_environment: public ::testing::Environment
{
public:
    virtual ~gc_environment() {}

    virtual void SetUp()
    {
        gc_init_options gc_ops;
        gc_ops.algo         = gc_algo::SERIAL;
        gc_ops.initiation   = gc_initiation::MANUAL;
        gc_ops.compacting   = gc_compacting::DISABLED;
        gc_ops.loglevel     = gc_loglevel::DEBUG;
        gc_ops.print_stat   = true;

        int res = gc_init(gc_ops);
        assert(res == 0);
    }

    virtual void TearDown() {}
};

::testing::Environment* const gc_env = ::testing::AddGlobalTestEnvironment(new gc_environment);