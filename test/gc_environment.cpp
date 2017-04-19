#include <gtest/gtest.h>

#include <cassert>

#include <liballocgc/liballocgc.hpp>

using namespace allocgc;

class gc_environment: public ::testing::Environment
{
public:
    virtual ~gc_environment() {}

    virtual void SetUp()
    {
        gc_params params;
        params.manual_init  = true;
        params.print_stat   = true;
        params.heapsize     = 128 * 1024 * 1024;
        params.loglevel     = gc_loglevel::DEBUG;

        int res = gc_init(params);
        assert(res == 0);
    }

    virtual void TearDown() {}
};

::testing::Environment* const gc_env = ::testing::AddGlobalTestEnvironment(new gc_environment);