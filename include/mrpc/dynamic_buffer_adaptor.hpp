#ifndef MRPC_DETAIL_DYNAMIC_BUFFER_HPP
#define MRPC_DETAIL_DYNAMIC_BUFFER_HPP

#include <any>
#include <boost/asio/buffer.hpp>
#include <proxy.h>

namespace mrpc {
namespace net = boost::asio;
using namespace std::placeholders;

class dynamic_buffer_adaptor
{
public:
    template<typename DynamicBuffer>
    dynamic_buffer_adaptor(DynamicBuffer &dynamic_buffer, std::enable_if_t<net::is_dynamic_buffer_v2<DynamicBuffer>::value, int *> = nullptr);

    ~dynamic_buffer_adaptor() = default;

    dynamic_buffer_adaptor(const dynamic_buffer_adaptor &) = default;

    dynamic_buffer_adaptor &operator=(const dynamic_buffer_adaptor &) = default;

    dynamic_buffer_adaptor(dynamic_buffer_adaptor &&) noexcept = default;

    dynamic_buffer_adaptor &operator=(dynamic_buffer_adaptor &&) noexcept = default;

public:
    [[nodiscard]] net::mutable_buffer data(size_t pos, size_t n);

    [[nodiscard]] net::const_buffer data(size_t pos, size_t n) const;

    void grow(size_t n);

    [[nodiscard]] size_t size() const;

    void shrink(size_t n);

private:
    struct dispatch_data : pro::dispatch<net::mutable_buffer(size_t, size_t)>
    {
        template<typename DynamicBuffer>
        net::mutable_buffer operator()(DynamicBuffer &buffer, size_t pos, size_t n)
        {
            return buffer.data(pos, n);
        }
    };

    struct dispatch_const_data : pro::dispatch<net::const_buffer(size_t, size_t)>
    {
        template<typename DynamicBuffer>
        net::const_buffer operator()(const DynamicBuffer &buffer, size_t pos, size_t n)
        {
            return buffer.data(pos, n);
        }
    };

    struct dispatch_grow : pro::dispatch<void(size_t)>
    {
        template<typename DynamicBuffer>
        void operator()(DynamicBuffer &buffer, size_t n)
        {
            buffer.grow(n);
        }
    };

    struct dispatch_size : pro::dispatch<size_t()>
    {
        template<typename DynamicBuffer>
        size_t operator()(DynamicBuffer &buffer)
        {
            return buffer.size();
        }
    };

    struct dispatch_shrink : pro::dispatch<void(size_t)>
    {
        template<typename DynamicBuffer>
        void operator()(DynamicBuffer &buffer, size_t n)
        {
            buffer.shrink(n);
        }
    };

    struct facade_dynamic_buffer : pro::facade<dispatch_data, dispatch_const_data, dispatch_grow, dispatch_size, dispatch_shrink>
    {
        static constexpr auto minimum_copyability = pro::constraint_level::trivial;
        static constexpr auto minimum_relocatability = pro::constraint_level::trivial;
        static constexpr auto minimum_destructibility = pro::constraint_level::trivial;
        static constexpr auto maximum_size = sizeof(void *);
        static constexpr auto maximum_alignment = alignof(void *);
    };

    pro::proxy<facade_dynamic_buffer> dynamic_buffer_;
};

template<typename DynamicBuffer>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(DynamicBuffer &dynamic_buffer, std::enable_if_t<net::is_dynamic_buffer_v2<DynamicBuffer>::value, int *>)
    : dynamic_buffer_(std::addressof(dynamic_buffer))
{}

} // namespace mrpc

#endif