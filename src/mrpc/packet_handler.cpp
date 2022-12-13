#include <mrpc/packet_handler.hpp>

namespace mrpc {

sys::error_code abstract_packet_handler::translate(sys::error_code ec)
{
    if (ec == net::error::operation_aborted || ec == net::experimental::error::channel_cancelled)
    {
        return rpc_error::operation_canceled;
    }

    if (ec == net::error::eof || ec == net::error::broken_pipe || ec == net::error::connection_reset || ec == net::experimental::error::channel_closed)
    {
        return rpc_error::connection_closed;
    }

    if (ec)
    {
        return ec;
    }

    return rpc_error::success;
}

net::awaitable<sys::error_code> channel_packet_handler::send(net::const_buffer packet) noexcept
{
    sys::error_code ec;

    container_type container;
    container.resize(packet.size());

    net::buffer_copy(net::buffer(container), packet);

    co_await out_.async_send({}, std::move(container), await_error(ec));
    co_return translate(ec);
}

net::awaitable<sys::error_code> channel_packet_handler::receive(dynamic_buffer_adaptor packet) noexcept
{
    sys::error_code ec;

    container_type container = co_await in_.async_receive(await_error(ec));
    if (ec)
    {
        co_return translate(ec);
    }

    auto pos = packet.size();
    packet.grow(container.size());

    net::buffer_copy(packet.data(pos, container.size()), net::buffer(container));

    co_return rpc_error::success;
}

net::awaitable<sys::error_code> channel_packet_handler::offer(net::const_buffer packet) noexcept
{
    sys::error_code ec;
    container_type container;
    container.resize(packet.size());

    net::buffer_copy(net::buffer(container), packet);
    co_await in_.async_send({}, std::move(container), await_error(ec));
    co_return translate(ec);
}

net::awaitable<sys::error_code> channel_packet_handler::poll(dynamic_buffer_adaptor packet) noexcept
{
    sys::error_code ec;

    container_type container = co_await out_.async_receive(await_error(ec));
    if (ec)
    {
        co_return translate(ec);
    }

    auto pos = packet.size();
    packet.grow(container.size());

    net::buffer_copy(packet.data(pos, container.size()), net::buffer(container));

    co_return rpc_error::success;
}

} // namespace mrpc