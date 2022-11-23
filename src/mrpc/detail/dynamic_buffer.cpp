#include <mrpc/detail/dynamic_buffer.hpp>

namespace mrpc::detail {
net::mutable_buffer dynamic_buffer::prepare(size_t n)
{
    return fn_prepare_(n);
}

void dynamic_buffer::commit(size_t n)
{
    fn_commit_(n);
}

} // namespace mrpc::detail