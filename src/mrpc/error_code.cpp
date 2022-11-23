#include <mrpc/error_code.hpp>

namespace mrpc {

const char *rpc_error_category_t::name() const noexcept
{
    return "mrpc::rpc_error";
}

std::string rpc_error_category_t::message(int code) const
{
    switch (static_cast<rpc_error>(code))
    {
    case rpc_error::success:
        return "success";
    case rpc_error::proto_serialize_fail:
        return "protobuf serialize fail";
    case rpc_error::proto_parse_fail:
        return "protobuf parse fail";
    case rpc_error::method_not_implement:
        return "method not implement";
    case rpc_error::unhandled_exception:
        return "exception occur";
    case rpc_error::data_corrupted:
        return "data corrupted";
    case rpc_error::operation_timeout:
        return "operation timeout";
    case rpc_error::connection_closed:
        return "connection closed";
    case rpc_error::operation_canceled:
        return "operation canceled";
    case rpc_error::unhandled_system_error:
        return "unhandled system error";
    default:
        return "unknown";
    }
}

boost::system::error_condition rpc_error_category_t::default_error_condition(int code) const noexcept
{
    switch (static_cast<rpc_error>(code))
    {
    case rpc_error::success:
        return {};
    default:
        return {code, *this};
    }
}

bool rpc_error_category_t::failed(int ev) const noexcept
{
    return static_cast<rpc_error>(ev) != rpc_error::success;
}

const rpc_error_category_t &rpc_error_category()
{
    static rpc_error_category_t instance;
    return instance;
}

boost::system::error_code make_error_code(rpc_error error)
{
    return {static_cast<int>(error), rpc_error_category()};
}
} // namespace mrpc