#ifndef MRPC_DETAIL_ERROR_CODE_H
#define MRPC_DETAIL_ERROR_CODE_H

#include <boost/system/error_code.hpp>

namespace mrpc::detail {

enum class system_error : int32_t
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

class system_error_category_impl : public boost::system::error_category
{
public:
    const char *name() const noexcept final;

    std::string message(int code) const final;

    boost::system::error_condition default_error_condition(int code) const noexcept final;

    bool failed(int ev) const noexcept override;
};

const system_error_category_impl &system_error_category();

boost::system::error_code make_error_code(system_error error);

} // namespace mrpc::detail

template<>
struct boost::system::is_error_code_enum<mrpc::detail::system_error> : std::true_type
{};

#endif