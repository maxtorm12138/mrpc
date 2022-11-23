#ifndef MRPC_ERROR_CODE_H
#define MRPC_ERROR_CODE_H

#include <boost/system/error_code.hpp>

namespace mrpc {

enum class rpc_error
{
    success = 0,
    proto_serialize_fail = 1,
    proto_parse_fail = 2,
    method_not_implement = 3,
    unhandled_exception = 4,
    data_corrupted = 5,
    operation_timeout = 6,
    connection_closed = 7,
    operation_canceled = 8,
    unhandled_system_error = 9
};

class rpc_error_category_t : public boost::system::error_category
{
public:
    const char *name() const noexcept final;

    std::string message(int code) const final;

    boost::system::error_condition default_error_condition(int code) const noexcept final;

    bool failed(int ev) const noexcept override;
};

const rpc_error_category_t &rpc_error_category();

boost::system::error_code make_error_code(rpc_error error);

} // namespace mrpc

template<>
struct boost::system::is_error_code_enum<mrpc::rpc_error> : std::true_type
{};

#endif