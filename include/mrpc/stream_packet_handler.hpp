#ifndef MRPC_STREAM_PACKET_HANDLER_HPP
#define MRPC_STREAM_PACKET_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <mrpc/packet_handler.hpp>

#include <boost/asio/deferred.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/recycling_allocator.hpp>

namespace mrpc {

template<typename AsyncReadWriteStream>
class stream_packet_handler final : public abstract_packet_handler
{
public:
    stream_packet_handler(AsyncReadWriteStream stream)
        : stream_(std::move(stream))
    {}

protected:
    void initiate_send(net::const_buffer packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept override;

    void initiate_receive(dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept override;

private:
    static sys::error_code translate(sys::error_code ec)
    {
        return ec;
    }

    AsyncReadWriteStream stream_;
    std::vector<uint8_t, net::recycling_allocator<uint8_t>> stream_read_buffer_;
};

template<typename AsyncReadWriteStream>
void stream_packet_handler<AsyncReadWriteStream>::initiate_send(net::const_buffer packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept
{
    sys::error_code ec;
    uint16_t size = packet.size();
    std::array<net::const_buffer, 2> buffers{net::buffer(&size, sizeof(size)), packet};
    net::async_write(stream_, buffers, net::deferred([](sys::error_code ec, size_t nwrite) { return net::deferred.values(translate(ec)); }))(std::move(handler));
}

template<typename AsyncReadWriteStream>
void stream_packet_handler<AsyncReadWriteStream>::initiate_receive(dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept
{
    stream_read_buffer_.clear();
    struct read_completion_condition
    {
        size_t operator()(sys::error_code ec, size_t n)
        {
            if (ec)
            {
                return 0;
            }

            switch (state)
            {
            case 1:
                if (goal - n == 0)
                {
                    uint16_t body_size = 0;
                    net::buffer_copy(net::buffer(&body_size, sizeof(body_size)), net::buffer(self->stream_read_buffer_, sizeof(body_size)));
                    goal += body_size;
                    state = 2;
                }
                break;
            case 2:
                break;
            }

            return goal - n;
        }

        stream_packet_handler<AsyncReadWriteStream> *self;
        size_t goal = sizeof(uint16_t);
        int state = 1;
    };

    net::async_read(stream_, net::dynamic_buffer(stream_read_buffer_), read_completion_condition{.self = this}, net::deferred([this, packet](sys::error_code ec, size_t n) mutable {
        if (ec)
        {
            return net::deferred.values(translate(ec));
        }

        auto pos = packet.size();
        size_t packet_size = stream_read_buffer_.size() - sizeof(uint16_t);
        packet.grow(packet_size);
        net::buffer_copy(packet.data(pos, packet_size), net::buffer(stream_read_buffer_.data() + sizeof(uint16_t), packet_size));
        stream_read_buffer_.clear();

        return net::deferred.values(mrpc::make_error_code(rpc_error::success));
    }))(std::move(handler));
}

} // namespace mrpc
#endif