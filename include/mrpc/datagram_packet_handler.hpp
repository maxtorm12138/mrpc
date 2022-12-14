#ifndef MRPC_STREAM_PACKET_HANDLER_HPP
#define MRPC_STREAM_PACKET_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <mrpc/packet_handler.hpp>

namespace mrpc {

template<typename AsyncDatagram>
class datagram_packet_handler final : public abstract_packet_handler
{
public:
    datagram_packet_handler(AsyncDatagram datagram)
        : datagram_(std::move(datagram))
    {}

protected:
    void initiate_send(net::const_buffer packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept override;

    void initiate_receive(dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept override;

private:
    AsyncDatagram datagram_;
};

template<typename AsyncDatagram>
void datagram_packet_handler<AsyncDatagram>::initiate_send(net::const_buffer packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept
{}

template<typename AsyncDatagram>
void datagram_packet_handler<AsyncDatagram>::initiate_receive(dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept
{}

} // namespace mrpc

#endif