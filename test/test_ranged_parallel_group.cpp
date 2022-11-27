#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <vector>
namespace net = boost::asio;

net::awaitable<void> timer_wait()
{
    try
    {
        using namespace std::chrono_literals;
        net::steady_timer timer0(co_await net::this_coro::executor);
        timer0.expires_after(10s);
        co_await timer0.async_wait(net::use_awaitable);
    }
    catch (std::exception &ex)
    {
        std::cout << "exception: " << ex.what() << std::endl;
    }
}

net::awaitable<void> co_main()
{
    auto executor = co_await net::this_coro::executor;
    using operation_type = decltype(net::co_spawn(executor, timer_wait(), net::deferred));

    std::vector<operation_type> ops;
    ops.emplace_back(net::co_spawn(executor, timer_wait(), net::deferred));
    ops.emplace_back(net::co_spawn(executor, timer_wait(), net::deferred));

    auto runner = net::experimental::make_parallel_group(std::move(ops));

    auto waiter = co_await runner.async_wait(net::experimental::wait_for_all(), net::deferred);
}

int main(int argc, char *argv[])
{
    net::io_context io_context;
    net::co_spawn(io_context, co_main(), net::detached);
    io_context.run();
}