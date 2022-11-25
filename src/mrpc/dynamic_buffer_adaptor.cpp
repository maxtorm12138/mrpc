#include <mrpc/dynamic_buffer_adaptor.hpp>

namespace mrpc {

dynamic_buffer_adaptor::dynamic_buffer_adaptor(std::nullptr_t)
    : dynamic_buffer_(nullptr)
{}

dynamic_buffer_adaptor::dynamic_buffer_adaptor(std::nullopt_t)
    : dynamic_buffer_(nullptr)
{}

net::mutable_buffer dynamic_buffer_adaptor::data(size_t pos, size_t n)
{
    return dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_data>(pos, n);
}

net::const_buffer dynamic_buffer_adaptor::data(size_t pos, size_t n) const
{
    return dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_const_data>(pos, n);
}

void dynamic_buffer_adaptor::grow(size_t n)
{
    dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_grow>(n);
}

size_t dynamic_buffer_adaptor::size() const
{
    return dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_size>();
}

void dynamic_buffer_adaptor::shrink(size_t n)
{
    dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_shrink>(n);
}

size_t dynamic_buffer_adaptor::capacity() const
{
    return dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_capacity>();
}

void dynamic_buffer_adaptor::consume(size_t n)
{
    return dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_consume>(n);
}

size_t dynamic_buffer_adaptor::max_size() const
{
    return dynamic_buffer_.invoke<detail::dispatch_dynamic_buffer_max_size>();
}

} // namespace mrpc