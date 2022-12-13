#ifndef MRPC_PACKET_HANDLER_H
#define MRPC_PACKET_HANDLER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/noncopyable.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/recycling_allocator.hpp>
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
    sys::error_code translate(sys::error_code ec);

public:
    virtual net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept = 0;

    virtual net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept = 0;
};

template<typename AsyncStream>
concept is_async_stream = requires(AsyncStream async_stream) {
                              requires std::is_nothrow_move_constructible_v<AsyncStream>;
                              {
                                  async_stream.get_executor()
                                  } -> std::convertible_to<net::any_io_executor>;
                              {
                                  async_stream.async_read_some(net::mutable_buffer(), net::use_awaitable)
                                  } -> std::same_as<net::awaitable<size_t>>;
                              {
                                  async_stream.async_write_some(net::const_buffer(), net::use_awaitable)
                                  } -> std::same_as<net::awaitable<size_t>>;
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
concept is_async_datagram = requires(AsyncDatagram async_datagram) {
                                requires std::is_nothrow_move_constructible_v<AsyncDatagram>;
                                {
                                    async_datagram.get_executor()
                                    } -> std::convertible_to<net::any_io_executor>;
                                {
                                    async_datagram.async_receive(net::mutable_buffer(), net::use_awaitable)
                                    } -> std::same_as<net::awaitable<size_t>>;
                                {
                                    async_datagram.async_send(net::const_buffer(), net::use_awaitable)
                                    } -> std::same_as<net::awaitable<size_t>>;
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

    net::awaitable<sys::error_code> send(net::const_buffer packet) noexcept override;

    net::awaitable<sys::error_code> receive(dynamic_buffer_adaptor packet) noexcept override;

    net::awaitable<sys::error_code> offer(net::const_buffer packet) noexcept;

    net::awaitable<sys::error_code> poll(dynamic_buffer_adaptor packet) noexcept;

private:
    using container_type = std::vector<uint8_t, net::recycling_allocator<uint8_t>>;
    using channel_type = net::experimental::channel<void(sys::error_code, container_type)>;
    channel_type in_;
    channel_type out_;
};

} // namespace mrpc

#include <mrpc/impl/packet_handler.hpp>

#endif