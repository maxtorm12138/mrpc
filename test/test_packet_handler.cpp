#include <mrpc/mrpc.hpp>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <mrpc/stream_packet_handler.hpp>
#include "test_utils.hpp"

class packet_handler_test : public testing::Test
{

protected:
    void SetUp() override {}
    void TearDown() override {}
};

net::awaitable<void> server()
{
    try
    {
        auto executor = co_await net::this_coro::executor;
        auto acceptor = new net::ip::tcp::acceptor(executor, net::ip::tcp::endpoint{net::ip::make_address("127.0.0.1"), 11451});
        net::co_spawn(
            executor,
            [acceptor]() -> net::awaitable<void> {
                auto socket = co_await acceptor->async_accept(net::use_awaitable);
                EXPECT_TRUE(socket.is_open());
                mrpc::stream_packet_handler packet_handler(std::move(socket));

                std::string buffer;
                co_await packet_handler.receive(buffer, net::use_awaitable);
                EXPECT_EQ(buffer, "hello from client");

                buffer = "server reply: " + buffer;
                co_await packet_handler.send(net::buffer(buffer), net::use_awaitable);
            }(),
            net::detached);
    }
    catch (std::exception &ex)
    {
        EXPECT_NO_THROW(throw ex);
    }
}

net::awaitable<void> client()
{
    try
    {
        net::ip::tcp::socket socket(co_await net::this_coro::executor);
        co_await socket.async_connect(net::ip::tcp::endpoint{net::ip::make_address("127.0.0.1"), 11451}, net::use_awaitable);

        mrpc::stream_packet_handler packet_handler(std::move(socket));

        std::string buffer = "hello from client";
        co_await packet_handler.send(net::buffer(buffer), net::use_awaitable);
        buffer.clear();

        co_await packet_handler.receive(buffer, net::use_awaitable);
        EXPECT_EQ(buffer, "server reply: hello from client");
    }
    catch (std::exception &ex)
    {
        EXPECT_NO_THROW(throw ex);
    }
}

MRPC_TEST_F(packet_handler_test, stream)
{
    auto executor = co_await net::this_coro::executor;

    co_await net::co_spawn(executor, server(), net::use_awaitable);
    co_await net::co_spawn(executor, client(), net::use_awaitable);
}
