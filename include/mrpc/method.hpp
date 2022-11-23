#ifndef MRPC_METHOD_H
#define MRPC_METHOD_H

// msft proxy
#include <proxy.h>

// boost
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/noncopyable.hpp>

#include <mrpc/detail/dynamic_buffer.hpp>

namespace mrpc {
namespace net = boost::asio;
namespace sys = boost::system;

class abstract_method : public boost::noncopyable
{
public:
    virtual net::awaitable<sys::error_code> operator()(net::const_buffer request, detail::dynamic_buffer &response) = 0;
};

} // namespace mrpc

#endif