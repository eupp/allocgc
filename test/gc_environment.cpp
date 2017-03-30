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
        gc_factory::options gc_ops;
        gc_ops.manual_init  = true;
        gc_ops.print_stat   = true;
        gc_ops.heapsize     = 128 * 1024 * 1024;
        gc_ops.loglevel     = gc_loglevel::DEBUG;

        int res = gc_init(gc_factory::create(gc_ops));
        assert(res == 0);
    }

    virtual void TearDown() {}
};

::testing::Environment* const gc_env = ::testing::AddGlobalTestEnvironment(new gc_environment);