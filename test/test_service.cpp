#include "test_utils.hpp"

class service_test : public testing::Test
{};

MRPC_TEST_F(service_test, implement_service)
{
    co_return;
}