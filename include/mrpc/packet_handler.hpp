#ifndef MRPC_PACKET_HANDLER_HPP
#define MRPC_PACKET_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/noncopyable.hpp>
#include <boost/asio/any_completion_handler.hpp>

#include <mrpc/dynamic_buffer_adaptor.hpp>
#include <mrpc/error_code.hpp>

namespace mrpc {

namespace net = boost::asio;
namespace sys = boost::system;

class abstract_packet_handler : public boost::noncopyable
{
public:
    virtual ~abstract_packet_handler() = default;

    template<net::completion_token_for<void(sys::error_code)> CompletionToken>
    auto async_send(net::const_buffer packet, CompletionToken &&token);

    template<net::completion_token_for<void(sys::error_code)> CompletionToken>
    auto async_receive(dynamic_buffer_adaptor packet, CompletionToken &&token);

protected:
    virtual void initiate_send(net::const_buffer packet, net::any_completion_handler<void(sys::error_code)>) noexcept = 0;

    virtual void initiate_receive(dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)>) noexcept = 0;
};

template<net::completion_token_for<void(sys::error_code)> CompletionToken>
auto abstract_packet_handler::async_send(net::const_buffer packet, CompletionToken &&token)
{
    auto initiation = [](net::completion_handler_for<void(sys::error_code)> auto completion_handler, abstract_packet_handler *self, net::const_buffer packet) {
        self->initiate_send(packet, std::move(completion_handler));
    };

    return net::async_initiate<CompletionToken, void(sys::error_code)>(initiation, token, this, packet);
}

template<net::completion_token_for<void(sys::error_code)> CompletionToken>
auto abstract_packet_handler::async_receive(dynamic_buffer_adaptor packet, CompletionToken &&token)
{
    auto initiation = [](net::completion_handler_for<void(sys::error_code)> auto completion_handler, abstract_packet_handler *self, dynamic_buffer_adaptor packet) {
        self->initiate_receive(packet, std::move(completion_handler));
    };

    return net::async_initiate<CompletionToken, void(sys::error_code)>(initiation, token, this, packet);
}

} // namespace mrpc

/*
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

*/

#endif