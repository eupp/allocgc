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
        enable_logging(gc_loglevel::DEBUG);

        serial::set_heap_limit(128 * 1024 * 1024);

        serial::register_main_thread();
    }

    virtual void TearDown() {}
};

::testing::Environment* const gc_env = ::testing::AddGlobalTestEnvironment(new gc_environment);