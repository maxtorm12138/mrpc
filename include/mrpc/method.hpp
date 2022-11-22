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

class abstract_method : public boost::noncopyable
{
public:
    virtual net::awaitable<sys::error_code> operator()(net::const_buffer request_payload) = 0;
};

} // namespace mrpc

#endif