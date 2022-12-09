#ifndef MRPC_PACKET_HANDLER_H
#define MRPC_PACKET_HANDLER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/noncopyable.hpp>

#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/basic_datagram_socket.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/awaitable.hpp>

#include <mrpc/dynamic_buffer_adaptor.hpp>
#include <mrpc/error_code.hpp>
#include <mrpc/await_error.hpp>
#include <boost/asio/recycling_allocator.hpp>

#if defined MRPC_ENABLE_SSL
#    include <botan/tls_version.h>
#    include <botan/tls_client.h>
#endif

namespace mrpc {

namespace net = boost::asio;
namespace sys = boost::system;

class abstract_packet_handler : public boost::noncopyable
{
public:
    virtual ~abstract_packet_handler() = default;

protected:
    sys::error_code translate(sys::error_code ec);

public:
    virtual net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept = 0;

    virtual net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept = 0;
};

template<typename AsyncStream>
concept is_async_stream = requires(AsyncStream stream_) {
                              requires std::is_nothrow_move_constructible_v<AsyncStream>;
                              requires std::is_nothrow_move_assignable_v<AsyncStream>;
                              requires net::is_executor<std::invoke_result_t<typename AsyncStream::get_executor>>::value;
                          };

template<is_async_stream AsyncStream>
class stream_packet_handler final : public abstract_packet_handler
{
public:
    stream_packet_handler(AsyncStream stream)
        : stream_(std::move(stream))
    {}

public:
    net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept override;

    net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept override;

private:
    AsyncStream stream_;
};

template<typename AsyncDatagram>
concept is_async_datagram = requires(AsyncDatagram datagram_) {
                                requires std::is_nothrow_move_constructible_v<AsyncDatagram>;
                                requires std::is_nothrow_move_assignable_v<AsyncDatagram>;
                                requires net::is_executor<std::invoke_result_t<typename AsyncDatagram::get_executor>>::value;
                            };

template<is_async_datagram AsyncDatagram>
class datagram_packet_handler : public abstract_packet_handler
{
public:
    datagram_packet_handler(AsyncDatagram datagram)
        : datagram_(std::move(datagram))
    {}

public:
    net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept override;

    net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept override;

private:
    AsyncDatagram datagram_;
};

class channel_packet_handler : public abstract_packet_handler
{

public:
    template<typename Executor>
    channel_packet_handler(const Executor &executor, size_t buffer = 100)
        : in_(std::forward<Executor>(executor), buffer)
        , out_(std::forward<Executor>(executor), buffer)
    {}

    net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept override
    {
        sys::error_code ec;

        container_type container;
        container.resize(packet.size());

        net::buffer_copy(net::buffer(container), packet);

        co_await out_.async_send({}, std::move(container), await_error(ec));
        co_return translate(ec);
    }

    net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept override
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

    net::awaitable<sys::error_code> offer(net::const_buffer packet)
    {
        sys::error_code ec;
        container_type container;
        container.resize(packet.size());

        net::buffer_copy(net::buffer(container), packet);
        co_await in_.async_send({}, std::move(container), await_error(ec));
        co_return translate(ec);
    }

    net::awaitable<sys::error_code> poll(dynamic_buffer_adaptor packet)
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

private:
    using container_type = std::vector<uint8_t, net::recycling_allocator<uint8_t>>;

    using channel_type = net::experimental::channel<void(sys::error_code, container_type)>;

    channel_type in_;
    channel_type out_;
};

#ifdef MRPC_ENABLE_SSL

#endif

} // namespace mrpc

#include <mrpc/impl/packet_handler.hpp>

#endif