#ifndef MRPC_DETAIL_AWAIT_ERROR_H
#define MRPC_DETAIL_AWAIT_ERROR_H
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/redirect_error.hpp>

namespace mrpc {
namespace net = boost::asio;
namespace sys = boost::system;

class await_error_t
{
public:
    auto operator()(sys::error_code &ec) const noexcept
    {
        return net::redirect_error(net::use_awaitable, ec);
    }

    auto operator()() const noexcept
    {
        return net::use_awaitable;
    }
};

constexpr await_error_t await_error;
} // namespace mrpc
#endif