#ifndef MRPC_IMPL_PACKET_HANDLER_HPP
#define MRPC_IMPL_PACKET_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <mrpc/packet_handler.hpp>

namespace mrpc {
template<is_async_stream AsyncStream>
net::awaitable<sys::error_code> stream_packet_handler<AsyncStream>::send(net::const_buffer packet) noexcept
{
    sys::error_code ec;
    uint16_t size = packet.size();
    std::array<net::const_buffer, 2> buffers{net::buffer(&size, sizeof(size)), packet};
    co_await net::async_write(stream_, buffers, await_error(ec));
    co_return translate(ec);
}

template<is_async_stream AsyncStream>
net::awaitable<sys::error_code> stream_packet_handler<AsyncStream>::receive(dynamic_buffer_adaptor packet) noexcept
{
    sys::error_code ec;

    uint16_t size = 0;
    co_await net::async_read(stream_, net::buffer(&size, sizeof(size)), await_error(ec));
    if (ec)
    {
        co_return translate(ec);
    }

    auto pos = packet.size();
    packet.grow(size);

    co_await net::async_read(stream_, packet.data(pos, size), await_error(ec));
    if (ec)
    {
        co_return translate(ec);
    }

    co_return rpc_error::success;
}

template<is_async_datagram AsyncDatagram>
net::awaitable<sys::error_code> datagram_packet_handler<AsyncDatagram>::send(net::const_buffer packet) noexcept
{
    sys::error_code ec;
    datagram_.async_send(packet, await_error(ec));
    co_return translate(ec);
}

template<is_async_datagram AsyncDatagram>
net::awaitable<sys::error_code> datagram_packet_handler<AsyncDatagram>::receive(dynamic_buffer_adaptor packet) noexcept
{
    sys::error_code ec;

    auto pos = packet.size();
    packet.grow(1500);
    auto buffer = packet.data(pos, 1500);
    size_t size = co_await datagram_.async_receive(buffer, await_error(ec));
    if (ec)
    {
        co_return translate(ec);
    }
    packet.shrink(pos + size);

    co_return rpc_error::success;
};
} // namespace mrpc

#endif