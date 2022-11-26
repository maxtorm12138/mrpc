#ifndef MRPC_AWAIT_ERROR_H
#define MRPC_AWAIT_ERROR_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/redirect_error.hpp>

namespace mrpc {
namespace net = boost::asio;
namespace sys = boost::system;

class await_error_t
{
public:
    net::redirect_error_t<net::use_awaitable_t<>> operator()(sys::error_code &ec) const noexcept;
    constexpr net::use_awaitable_t<> operator()() const noexcept;
};

constexpr await_error_t await_error;
} // namespace mrpc
#endif