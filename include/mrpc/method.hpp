#ifndef MRPC_METHOD_H
#define MRPC_METHOD_H

// msft proxy
#include <proxy.h>

// boost
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/noncopyable.hpp>

// protobuf
#include <google/protobuf/message.h>

namespace mrpc {
namespace net = boost::asio;
namespace sys = boost::system;

struct dynamic_buffer_prepare : pro::dispatch<net::mutable_buffer(size_t)>
{
    template<typename DynamicBuffer>
    net::mutable_buffer operator()(DynamicBuffer &dynamic_buffer, size_t n)
    {
        return dynamic_buffer.prepare(n);
    }
};

struct dynamic_buffer_commit : pro::dispatch<void(size_t)>
{
    template<typename DynamicBuffer>
    void operator()(DynamicBuffer &dynamic_buffer, size_t n)
    {
        dynamic_buffer.commit(n);
    }
};

struct dynamic_buffer_facade : pro::facade<dynamic_buffer_prepare, dynamic_buffer_commit>
{};

class abstract_method : public boost::noncopyable
{
public:
    virtual net::awaitable<sys::error_code> operator()(net::const_buffer request, pro::proxy<dynamic_buffer_facade> &response) = 0;
};

} // namespace mrpc

#endif