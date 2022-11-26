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

namespace mrpc {

namespace net = boost::asio;
namespace sys = boost::system;

class abstract_packet_handler : public boost::noncopyable
{
public:
    virtual ~abstract_packet_handler() = default;

protected:
    static sys::error_code translate(sys::error_code ec)
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

public:
    virtual net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept = 0;

    virtual net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept = 0;
};

template<typename Protocol, typename Executor>
class stream_packet_handler final : public abstract_packet_handler
{
public:
    stream_packet_handler(net::basic_stream_socket<Protocol, Executor> socket)
        : socket_(std::move(socket))
    {}

public:
    net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept override
    {
        sys::error_code ec;

        uint16_t size = packet.size();
        std::array<net::const_buffer, 2> buffers{net::buffer(&size, sizeof(size)), packet};
        co_await net::async_write(socket_, buffers, await_error(ec));
        co_return translate(ec);
    }

    net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept override
    {
        sys::error_code ec;

        uint16_t size = 0;
        co_await net::async_read(socket_, net::buffer(&size, sizeof(size)), await_error(ec));
        if (ec)
        {
            co_return translate(ec);
        }

        auto pos = packet.size();
        packet.grow(size);

        co_await net::async_read(socket_, packet.data(pos, size), await_error(ec));
        if (ec)
        {
            co_return translate(ec);
        }

        co_return rpc_error::success;
    }

private:
    net::basic_stream_socket<Protocol, Executor> socket_;
};

template<typename Protocol, typename Executor>
class datagram_packet_handler : public abstract_packet_handler
{
public:
    datagram_packet_handler(net::basic_datagram_socket<Protocol, Executor> socket)
        : socket_(std::move(socket))
    {}

public:
    net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept override
    {
        sys::error_code ec;
        socket_.async_send(packet, await_error(ec));
        co_return translate(ec);
    }

    net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept override
    {
        sys::error_code ec;

        auto pos = packet.size();
        packet.grow(1500);
        auto buffer = packet.data(pos, 1500);
        size_t size = co_await socket_.async_receive(buffer, await_error(ec));
        if (ec)
        {
            co_return translate(ec);
        }
        packet.shrink(pos + size);

        co_return rpc_error::success;
    };

private:
    net::basic_datagram_socket<Protocol, Executor> socket_;
};

template<typename Container, typename Channel = net::experimental::channel<void(sys::error_code, Container)>>
class channel_packet_handler : public abstract_packet_handler
{
public:
    template<typename Executor>
    channel_packet_handler(Executor &&executor, size_t buffer = 100)
        : in_(std::forward<Executor>(executor), buffer)
        , out_(std::forward<Executor>(executor), buffer)
    {}

    net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept override
    {
        sys::error_code ec;

        Container container;
        container.resize(packet.size());

        net::buffer_copy(net::buffer(container), packet);

        co_await out_.async_send({}, std::move(container), await_error(ec));
        co_return translate(ec);
    }

    net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept override
    {
        sys::error_code ec;

        Container container = co_await in_.async_receive(await_error(ec));
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
        Container container;
        container.resize(packet.size());

        net::buffer_copy(net::buffer(container), packet);
        co_await in_.async_send({}, std::move(container), await_error(ec));
        co_return translate(ec);
    }

    net::awaitable<sys::error_code> poll(dynamic_buffer_adaptor packet)
    {
        sys::error_code ec;

        Container container = co_await out_.async_receive(await_error(ec));
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
    Channel in_;
    Channel out_;
};

} // namespace mrpc

#endif