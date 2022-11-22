#include "mrpc/detail/error_code.hpp"

namespace mrpc::detail {

const char *system_error_category_impl::name() const noexcept
{
    return "mrpc::system_error";
}

std::string system_error_category_impl::message(int code) const
{
    switch (static_cast<system_error>(code))
    {
    case system_error::success:
        return "success";
    case system_error::proto_serialize_fail:
        return "protobuf serialize fail";
    case system_error::proto_parse_fail:
        return "protobuf parse fail";
    case system_error::method_not_implement:
        return "method not implement";
    case system_error::unhandled_exception:
        return "exception occur";
    case system_error::data_corrupted:
        return "data corrupted";
    case system_error::operation_timeout:
        return "operation timeout";
    case system_error::connection_closed:
        return "connection closed";
    case system_error::operation_canceled:
        return "operation canceled";
    case system_error::unhandled_system_error:
        return "unhandled system error";
    default:
        return "unknown";
    }
}

boost::system::error_condition system_error_category_impl::default_error_condition(int code) const noexcept
{
    switch (static_cast<system_error>(code))
    {
    case system_error::success:
        return {};
    default:
        return {code, *this};
    }
}

bool system_error_category_impl::failed(int ev) const noexcept
{
    return static_cast<system_error>(ev) != system_error::success;
}

const system_error_category_impl &system_error_category()
{
    static system_error_category_impl instance;
    return instance;
}

boost::system::error_code make_error_code(system_error error)
{
    return {static_cast<int>(error), system_error_category()};
}
} // namespace mrpc::detail