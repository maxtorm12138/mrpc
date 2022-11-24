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

    std::string msg = "hello from other side";
    co_await handler.offer(net::buffer(msg));

    std::string received_msg;
    auto buffer_received_msg = net::dynamic_buffer(received_msg);

    co_await handler.receive(buffer_received_msg);
    EXPECT_EQ(received_msg, msg);

    std::string msg1 = "at least I can say that I've tried";
    co_await handler.send(net::buffer(msg1));

    std::string received_msg1;
    auto buffer_received_msg1 = net::dynamic_buffer(received_msg1);
    co_await handler.poll(buffer_received_msg1);
    EXPECT_EQ(received_msg1, msg1);
}
