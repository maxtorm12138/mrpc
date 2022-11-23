#include <mrpc/await_error.hpp>

namespace mrpc {

net::redirect_error_t<net::use_awaitable_t<>> await_error_t::operator()(sys::error_code &ec) const noexcept
{
    return net::redirect_error(net::use_awaitable, ec);
}

constexpr net::use_awaitable_t<> await_error_t::operator()() const noexcept
{
    return net::use_awaitable;
}
} // namespace mrpc