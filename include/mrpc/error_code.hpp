#ifndef MRPC_ERROR_CODE_H
#define MRPC_ERROR_CODE_H

#include <mrpc/detail/error_code.hpp>

namespace mrpc {
using detail::make_error_code;
using detail::system_error;
} // namespace mrpc
#endif