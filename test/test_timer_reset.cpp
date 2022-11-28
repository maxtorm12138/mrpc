#include "test_utils.hpp"
#include <boost/asio/experimental/parallel_group.hpp>

namespace sys = boost::system;
class timer_reset_test : public testing::Test
{};

MRPC_TEST_F(timer_reset_test, external_cancel)
{
    using namespace std::chrono_literals;

    auto executor = co_await net::this_coro::executor;

    net::steady_timer timer(executor);

    auto canceller = [&timer]() -> net::awaitable<void> {
        auto executor = co_await net::this_coro::executor;
        net::steady_timer timer1(executor);

        timer1.expires_after(500ms);
        co_await timer1.async_wait(net::use_awaitable);

        timer.cancel();
    };

    auto waiter = [&timer]() -> net::awaitable<void> {
        auto state = co_await net::this_coro::cancellation_state;
        timer.expires_after(1000s);
        sys::error_code ec;
        co_await timer.async_wait(net::redirect_error(net::use_awaitable, ec));
        EXPECT_EQ(ec, net::error::operation_aborted);
        EXPECT_EQ(state.cancelled(), net::cancellation_type::none);
    };

    net::co_spawn(executor, canceller(), net::detached);

    auto result =
        co_await net::experimental::make_parallel_group(net::co_spawn(executor, waiter(), net::deferred)).async_wait(net::experimental::wait_for_all(), net::use_awaitable);
}

MRPC_TEST_F(timer_reset_test, internal_cancel)
{
    using namespace std::chrono_literals;

    auto executor = co_await net::this_coro::executor;

    net::steady_timer timer(executor);

    auto canceller = []() -> net::awaitable<void> {
        auto executor = co_await net::this_coro::executor;
        net::steady_timer timer1(executor);

        timer1.expires_after(500ms);
        co_await timer1.async_wait(net::use_awaitable);
    };

    auto waiter = [&timer]() -> net::awaitable<void> {
        auto state = co_await net::this_coro::cancellation_state;
        timer.expires_after(1000s);
        sys::error_code ec;
        co_await timer.async_wait(net::redirect_error(net::use_awaitable, ec));
        EXPECT_EQ(ec, net::error::operation_aborted);
        EXPECT_GT(state.cancelled(), net::cancellation_type::none);
    };

    net::co_spawn(executor, canceller(), net::detached);

    auto result = co_await net::experimental::make_parallel_group(net::co_spawn(executor, waiter(), net::deferred), net::co_spawn(executor, canceller(), net::deferred))
                      .async_wait(net::experimental::wait_for_one(), net::use_awaitable);
}