#include <mrpc/dynamic_buffer_adaptor.hpp>

namespace mrpc {

net::mutable_buffer dynamic_buffer_adaptor::data(size_t pos, size_t n)
{
    return dynamic_buffer_.invoke<dispatch_data>(pos, n);
}

net::const_buffer dynamic_buffer_adaptor::data(size_t pos, size_t n) const
{
    return dynamic_buffer_.invoke<dispatch_const_data>(pos, n);
}

void dynamic_buffer_adaptor::grow(size_t n)
{
    dynamic_buffer_.invoke<dispatch_grow>(n);
}

size_t dynamic_buffer_adaptor::size() const
{
    return dynamic_buffer_.invoke<dispatch_size>();
}

void dynamic_buffer_adaptor::shrink(size_t n)
{
    dynamic_buffer_.invoke<dispatch_shrink>(n);
}

} // namespace mrpc