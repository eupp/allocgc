#include <gtest/gtest.h>

#include <cassert>

#include "libprecisegc/libprecisegc.h"

class gc_environment: public ::testing::Environment
{
public:
    virtual ~gc_environment() {}

    virtual void SetUp()
    {
        int res = precisegc::gc_init();
        assert(res == 0);
    }

    virtual void TearDown() {}
};

::testing::Environment* const gc_env = ::testing::AddGlobalTestEnvironment(new gc_environment);