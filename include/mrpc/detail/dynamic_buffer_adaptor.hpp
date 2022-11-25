#ifndef MRPC_DETAIL_DYNAMIC_BUFFER_HPP
#define MRPC_DETAIL_DYNAMIC_BUFFER_HPP

#include <proxy.h>

#include <boost/asio/buffer.hpp>

namespace mrpc::detail {
namespace net = boost::asio;

struct dispatch_dynamic_buffer_data : pro::dispatch<net::mutable_buffer(size_t, size_t)>
{
    template<typename DynamicBuffer>
    net::mutable_buffer operator()(DynamicBuffer &buffer, size_t pos, size_t n)
    {
        return buffer.data(pos, n);
    }
};

struct dispatch_dynamic_buffer_const_data : pro::dispatch<net::const_buffer(size_t, size_t)>
{
    template<typename DynamicBuffer>
    net::const_buffer operator()(const DynamicBuffer &buffer, size_t pos, size_t n)
    {
        return buffer.data(pos, n);
    }
};

struct dispatch_dynamic_buffer_grow : pro::dispatch<void(size_t)>
{
    template<typename DynamicBuffer>
    void operator()(DynamicBuffer &buffer, size_t n)
    {
        buffer.grow(n);
    }
};

struct dispatch_dynamic_buffer_size : pro::dispatch<size_t()>
{
    template<typename DynamicBuffer>
    size_t operator()(const DynamicBuffer &buffer)
    {
        return buffer.size();
    }
};

struct dispatch_dynamic_buffer_shrink : pro::dispatch<void(size_t)>
{
    template<typename DynamicBuffer>
    void operator()(DynamicBuffer &buffer, size_t n)
    {
        buffer.shrink(n);
    }
};

struct dispatch_dynamic_buffer_capacity : pro::dispatch<size_t()>
{
    template<typename DynamicBuffer>
    size_t operator()(const DynamicBuffer &buffer)
    {
        return buffer.capacity();
    }
};

struct dispatch_dynamic_buffer_consume : pro::dispatch<void(size_t)>
{
    template<typename DynamicBuffer>
    void operator()(DynamicBuffer &buffer, size_t n)
    {
        buffer.consume(n);
    }
};

struct dispatch_dynamic_buffer_max_size : pro::dispatch<size_t()>
{
    template<typename DynamicBuffer>
    size_t operator()(const DynamicBuffer &buffer)
    {
        return buffer.max_size();
    }
};

struct facade_dynamic_buffer
    : pro::facade<dispatch_dynamic_buffer_data, dispatch_dynamic_buffer_data, dispatch_dynamic_buffer_const_data, dispatch_dynamic_buffer_grow, dispatch_dynamic_buffer_size,
          dispatch_dynamic_buffer_shrink, dispatch_dynamic_buffer_capacity, dispatch_dynamic_buffer_consume, dispatch_dynamic_buffer_max_size>
{
    static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
    static constexpr auto minimum_relocatability = pro::constraint_level::nothrow;
    static constexpr auto minimum_destructibility = pro::constraint_level::nothrow;
    static constexpr auto maximum_size = sizeof(void *) * 4u;
    static constexpr auto maximum_alignment = alignof(void *);
};

} // namespace mrpc::detail
#endif