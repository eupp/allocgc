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
        gc_options gc_ops;
        gc_ops.type         = gc_type::SERIAL;
        gc_ops.compacting   = gc_compacting::ENABLED;
        gc_ops.loglevel     = gc_loglevel::DEBUG;
        gc_ops.print_stat   = true;

        int res = gc_init(gc_ops);
        assert(res == 0);
    }

    virtual void TearDown() {}
};

::testing::Environment* const gc_env = ::testing::AddGlobalTestEnvironment(new gc_environment);