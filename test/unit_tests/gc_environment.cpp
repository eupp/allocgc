#include <gtest/gtest.h>

#include <cassert>

#include "libprecisegc/libprecisegc.h"

using namespace precisegc;

class gc_environment: public ::testing::Environment
{
public:
    virtual ~gc_environment() {}

    virtual void SetUp()
    {
        gc_options gc_ops;
        gc_ops.strategy   = gc_strategy::SERIAL;
        gc_ops.compacting = gc_compacting::ENABLED;
        int res = gc_init(gc_ops);
        assert(res == 0);
    }

    virtual void TearDown() {}
};

::testing::Environment* const gc_env = ::testing::AddGlobalTestEnvironment(new gc_environment);