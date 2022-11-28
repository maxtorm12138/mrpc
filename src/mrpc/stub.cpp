#include <mrpc/stub.hpp>

#include <numeric>

#include <boost/asio/deferred.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <spdlog/spdlog.h>
namespace mrpc {

const boost::uuids::uuid &stub::id() const
{
    return id_;
}

abstract_packet_handler &stub::packet_handler()
{
    return *packet_handler_;
}

net::awaitable<void> stub::async_run()
{

    auto executor = co_await net::this_coro::executor;
    using operation_type = decltype(net::co_spawn(executor, launch(0), net::deferred));

    std::vector<operation_type> stub_operations;
    stub_operations.reserve(option_.worker + 2);

    for (size_t id = 0; id < option_.worker + 2; id++)
    {
        stub_operations.emplace_back(net::co_spawn(executor, launch(id), net::deferred));
    }

    auto runner = net::experimental::make_parallel_group(std::move(stub_operations));

    co_await runner.async_wait(net::experimental::wait_for_one(), net::use_awaitable);
}

net::awaitable<void> stub::launch(size_t launch_id)
{
    auto state = co_await net::this_coro::cancellation_state;
    SPDLOG_TRACE("{}:{} launch started", boost::uuids::to_string(id_), launch_id);
    BOOST_SCOPE_EXIT_ALL(&)
    {
        SPDLOG_TRACE("{}:{} launch stopeed", boost::uuids::to_string(id_), launch_id);
    };

    while (!state.cancelled())
    {
        if (launch_id == 0)
        {
            co_await input_logic(launch_id);
        }
        else if (launch_id == 0)
        {
            co_await output_logic(launch_id);
        }
        else
        {
            co_await worker_logic(launch_id);
        }
    }
}

net::awaitable<void> stub::input_logic(size_t launch_id)
{
    container_type<uint8_t> buffer;
    auto ec = co_await packet_handler_->receive(buffer);
    if (ec)
    {
        throw sys::system_error(ec);
    }

    co_await worker_.async_send({}, std::move(buffer), await_error(ec));
    if (ec)
    {
        throw sys::system_error(ec);
    }
}

net::awaitable<void> stub::output_logic(size_t launch_id)
{
    sys::error_code ec;
    auto buffer = co_await out_.async_receive(await_error(ec));
    if (ec)
    {
        throw sys::system_error(ec);
    }

    ec = co_await packet_handler_->send(net::buffer(buffer));
    if (ec)
    {
        throw sys::system_error(ec);
    }
}

net::awaitable<void> stub::worker_logic(size_t launch_id)
{
    sys::error_code ec;
    auto buffer = co_await worker_.async_receive(await_error(ec));
    if (ec)
    {
        throw sys::system_error(ec);
    }

    auto [context, message] = co_await unpack(net::buffer(buffer));
    SPDLOG_DEBUG("{}:{} worker_logic unpack context: [{}]", boost::uuids::to_string(id_), launch_id, context.ShortDebugString());

    if (context.flags().contains(mrpc::MessageFlag::REQUEST))
    {
        container_type<uint8_t> response_message;
        auto ec = co_await service_->invoke(context, message, response_message);

        if (context.flags().contains(mrpc::MessageFlag::NO_REPLY))
        {
            co_return;
        }

        container_type<uint8_t> response;
        co_await pack(context, net::buffer(response), response);

        co_await out_.async_send({}, std::move(response), await_error(ec));
    }
    else if (context.flags().contains(mrpc::MessageFlag::RESPONSE))
    {
        auto call = call_.extract(context.trace_id());
        if (call.empty())
        {
            SPDLOG_INFO("{}:{} worker_logic ", boost::uuids::to_string(id_), launch_id);
            co_return;
        }

        co_await call.mapped()->async_send({}, std::move(buffer), await_error(ec));
    }
    else {}
}

net::awaitable<std::tuple<mrpc::Context, net::const_buffer>> stub::unpack(net::const_buffer buffer)
{
    uint8_t cmd_id = 0;
    uint32_t flags = 0;
    uint32_t error_code = 0;
    std::string trace_id(16, '\0');

    std::array header{net::buffer(&cmd_id, sizeof(cmd_id)), net::buffer(&flags, sizeof(flags)), net::buffer(&error_code, sizeof(error_code)), net::buffer(trace_id, 16)};

    buffer += net::buffer_copy(header, buffer);

    Context context;
    context.set_cmd_id(cmd_id);
    context.set_error_code(error_code);
    context.set_trace_id(std::string(trace_id.begin(), trace_id.end()));

    std::bitset<32> bit_flags(flags);
    for (int i = 0; i < MessageFlag::MESSAGE_FLAG_END; i++)
    {
        if (bit_flags.test(i))
        {
            context.mutable_flags()->emplace(i, 1);
        }
    }

    co_return std::make_tuple(context, buffer);
}

net::awaitable<void> stub::pack(const mrpc::Context &context, net::const_buffer message, dynamic_buffer_adaptor buffer)
{
    uint8_t cmd_id = context.cmd_id();
    uint32_t flags = 0;
    uint32_t error_code = context.error_code();
    std::string trace_id = context.trace_id();

    std::bitset<32> bit_flags{};
    for (int i = 0; i < MessageFlag::MESSAGE_FLAG_END; i++)
    {
        if (auto it_flag = context.flags().find(i); it_flag != context.flags().end() && it_flag->second == 1)
        {
            bit_flags.set(i, true);
        }
        else
        {
            bit_flags.set(i, false);
        }
    }

    flags = bit_flags.to_ulong();

    auto pos = buffer.size();

    // clang-format off
    container_type<net::const_buffer> template_buffer {
        net::buffer(&cmd_id, sizeof(cmd_id)),
        net::buffer(&flags, sizeof(flags)),
        net::buffer(&error_code, sizeof(error_code)),
        net::buffer(trace_id, 16), message
    };
    // clang-format on

    uint64_t size_to_grow = std::accumulate(template_buffer.begin(), template_buffer.end(), 0ULL, [](size_t curr, net::const_buffer buffer) { return curr + buffer.size(); });
    buffer.grow(size_to_grow);

    net::buffer_copy(buffer.data(pos, size_to_grow), template_buffer);

    co_return;
}

} // namespace mrpc