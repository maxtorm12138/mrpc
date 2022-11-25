#include <mrpc/mrpc.hpp>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "test_utils.hpp"

class packet_handler_test : public testing::Test
{

protected:
    void SetUp() override {}
    void TearDown() override {}
};

MRPC_TEST_F(packet_handler_test, channel)
{
    mrpc::channel_packet_handler<std::vector<uint8_t>> handler(co_await net::this_coro::executor);

    std::string msg = "hello from outside";
    auto ec = co_await handler.offer(net::buffer(msg));
    EXPECT_FALSE(!!ec);

    std::string received_msg;
    ec = co_await handler.receive(received_msg);
    EXPECT_FALSE(!!ec);

    EXPECT_EQ(received_msg, msg);

    std::string msg1 = "at least I can say that I've tried";
    ec = co_await handler.send(net::buffer(msg1));
    EXPECT_FALSE(!!ec);

    std::string received_msg1;
    ec = co_await handler.poll(received_msg1);
    EXPECT_FALSE(!!ec);

    EXPECT_EQ(received_msg1, msg1);
}
