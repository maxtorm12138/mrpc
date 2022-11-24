#include <mrpc/mrpc.hpp>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
namespace net = boost::asio;

class PacketHandlerTest : public testing::Test
{

protected:
    void SetUp() override
    {
        io_context_ = std::make_unique<net::io_context>();
    }

    void TearDown() override
    {
        io_context_ = nullptr;
    }

    void RunEntry(net::awaitable<void> entry)
    {
        net::co_spawn(io_context_->get_executor(), std::move(entry), [](std::exception_ptr ex) {
            if (ex != nullptr)
            {
                std::rethrow_exception(ex);
            }
        });
    }

    std::unique_ptr<net::io_context> io_context_;
};

TEST_F(PacketHandlerTest, ChannelSend)
{
    auto tester = []() -> net::awaitable<void> {
        mrpc::channel_packet_handler<std::vector<uint8_t>> handler(co_await net::this_coro::executor);

        std::string msg = "hello from other side";
        co_await handler.deliver(net::buffer(msg));

        std::string received_msg;
        auto buffer_received_msg = net::dynamic_buffer(received_msg);

        co_await handler.receive(buffer_received_msg);
        EXPECT_EQ(received_msg, msg);

        std::string msg1 = "At least I can say that I've tried";
        co_await handler.send(net::buffer(msg1));

        std::string received_msg1;
        auto buffer_received_msg1 = net::dynamic_buffer(received_msg1);
        co_await handler.retain(buffer_received_msg1);
        EXPECT_EQ(received_msg1, msg1);

        EXPECT_EQ(handler.in_.capacity(), 100);
        EXPECT_EQ(handler.out_.capacity(), 100);

        EXPECT_FALSE(handler.in_.ready());
        EXPECT_FALSE(handler.out_.ready());
    };
    RunEntry(tester());
}
