#include <mrpc/dynamic_buffer_adaptor.hpp>

namespace mrpc {

net::mutable_buffer dynamic_buffer_adaptor::data(size_t pos, size_t n)
{
    return data_(pos, n);
}

net::const_buffer dynamic_buffer_adaptor::data(size_t pos, size_t n) const
{
    return const_data_(pos, n);
}

void dynamic_buffer_adaptor::grow(size_t n)
{
    return grow_(n);
};

} // namespace mrpc