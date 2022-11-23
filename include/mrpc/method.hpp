#ifndef MRPC_METHOD_H
#define MRPC_METHOD_H

// msft proxy
#include <proxy.h>

// boost
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/noncopyable.hpp>

#include <mrpc/dynamic_buffer_adaptor.hpp>

namespace mrpc {
namespace net = boost::asio;
namespace sys = boost::system;

class abstract_method : public boost::noncopyable
{
public:
    virtual net::awaitable<sys::error_code> operator()(net::const_buffer request, dynamic_buffer_adaptor response) = 0;
};

} // namespace mrpc

#endif