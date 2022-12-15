#ifndef MRPC_STREAM_PACKET_HANDLER_HPP
#define MRPC_STREAM_PACKET_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <mrpc/packet_handler.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace mrpc::detail {
template<typename AsyncReadWriteStream>
class async_send_op
{
public:
    async_send_op(AsyncReadWriteStream &stream, net::const_buffer packet, net::any_completion_handler<void(sys::error_code)> handler)
        : state_(op_state::head)
        , stream_(stream)
        , packet_(packet)
        , handler_(std::move(handler))
        , size_(std::allocate_shared<uint16_t>(net::recycling_allocator<uint16_t>(), static_cast<uint16_t>(packet.size())))
    {
        (*this)({}, 0);
    }

    void operator()(sys::error_code ec, size_t)
    {
        switch (state_)
        {
        case op_state::head:
            state_ = op_state::body;
            net::async_write(stream_, net::buffer(size_.get(), sizeof(*size_)), std::move(*this));
            break;
        case op_state::body:
            if (ec)
            {
                std::move(handler_)(ec);
            }
            else
            {
                state_ = op_state::done;
                net::async_write(stream_, packet_, std::move(*this));
            }
            break;
        case op_state::done:
            std::move(handler_)(ec);
            break;
        }
    }

private:
    enum class op_state
    {
        head,
        body,
        done
    };
    op_state state_;
    AsyncReadWriteStream &stream_;
    net::const_buffer packet_;
    net::any_completion_handler<void(sys::error_code)> handler_;
    std::shared_ptr<uint16_t> size_;
};

template<typename AsyncReadWriteStream>
class async_receive_op
{
public:
    async_receive_op(AsyncReadWriteStream &stream, dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)> handler)
        : state_(op_state::head)
        , stream_(stream)
        , packet_(packet)
        , handler_(std::move(handler))
        , size_(std::allocate_shared<uint16_t>(net::recycling_allocator<uint16_t>(), 0))
    {
        (*this)({}, 0);
    }

    void operator()(sys::error_code ec, size_t)
    {
        switch (state_)
        {
        case op_state::head:
            state_ = op_state::body;
            net::async_read(stream_, net::buffer(size_.get(), sizeof(*size_)), std::move(*this));
            break;
        case op_state::body:
            if (ec)
            {
                std::move(handler_)(ec);
            }
            else
            {
                state_ = op_state::done;
                net::async_read(stream_, packet_, net::transfer_exactly(*size_), std::move(*this));
            }
            break;
        case op_state::done:
            std::move(handler_)(ec);
            break;
        }
    }

private:
    enum class op_state
    {
        head,
        body,
        done
    };
    op_state state_ = op_state::head;
    AsyncReadWriteStream &stream_;
    dynamic_buffer_adaptor packet_;
    net::any_completion_handler<void(sys::error_code)> handler_;
    std::shared_ptr<uint16_t> size_;
};
} // namespace mrpc::detail

namespace mrpc {
template<typename AsyncReadWriteStream>
class stream_packet_handler final : public abstract_packet_handler
{
public:
    stream_packet_handler(AsyncReadWriteStream stream)
        : stream_(std::move(stream))
        , strand_(stream.get_executor())
    {}

protected:
    void initiate_send(net::const_buffer packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept override;

    void initiate_receive(dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept override;

private:
    constexpr static sys::error_code translate(sys::error_code ec)
    {
        if (ec == net::error::operation_aborted)
        {
            return rpc_error::operation_canceled;
        }

        if (ec == net::error::eof || ec == net::error::broken_pipe || ec == net::error::connection_reset)
        {
            return rpc_error::connection_closed;
        }

        if (ec)
        {
            return rpc_error::unhandled_system_error;
        }

        return rpc_error::success;
    }

    AsyncReadWriteStream stream_;
    net::strand<typename AsyncReadWriteStream::executor_type> strand_;
};

template<typename AsyncReadWriteStream>
void stream_packet_handler<AsyncReadWriteStream>::initiate_send(net::const_buffer packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept
{
    detail::async_send_op<AsyncReadWriteStream>(stream_, packet, std::move(handler));
}

template<typename AsyncReadWriteStream>
void stream_packet_handler<AsyncReadWriteStream>::initiate_receive(dynamic_buffer_adaptor packet, net::any_completion_handler<void(sys::error_code)> handler) noexcept
{
    detail::async_receive_op<AsyncReadWriteStream>(stream_, packet, std::move(handler));
}

} // namespace mrpc
#endif