#ifndef MRPC_DYNAMIC_BUFFER_HPP
#define MRPC_DYNAMIC_BUFFER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/buffer.hpp>

#include <proxy.h>

#include <mrpc/detail/dynamic_buffer_adaptor.hpp>

namespace mrpc {
namespace net = boost::asio;

class dynamic_buffer_adaptor;
template<typename DynamicBuffer>
concept is_dynamic_buffer = requires {
                                requires net::is_dynamic_buffer<DynamicBuffer>::value;
                                requires !std::same_as<DynamicBuffer, dynamic_buffer_adaptor>;
                            };

template<typename DynamicBuffer>
concept is_small_dynamic_buffer = requires {
                                      requires is_dynamic_buffer<DynamicBuffer>;
                                      requires pro::proxiable<pro::details::sbo_ptr<DynamicBuffer>, detail::facade_dynamic_buffer>;
                                  };

class dynamic_buffer_adaptor
{
public:
    template<is_dynamic_buffer DynamicBuffer>
    dynamic_buffer_adaptor(DynamicBuffer *dynamic_buffer);

    template<is_dynamic_buffer DynamicBuffer>
    dynamic_buffer_adaptor(DynamicBuffer &dynamic_buffer);

    template<is_small_dynamic_buffer DynamicBuffer>
        requires(!std::is_reference_v<DynamicBuffer>)
    dynamic_buffer_adaptor(DynamicBuffer &&dynamic_buffer);

    template<typename Elem, typename Allocator>
    dynamic_buffer_adaptor(std::vector<Elem, Allocator> &dynamic_buffer);

    template<typename Elem, typename Traits, typename Allocator>
    dynamic_buffer_adaptor(std::basic_string<Elem, Traits, Allocator> &dynamic_buffer);

    dynamic_buffer_adaptor(std::nullptr_t);

    dynamic_buffer_adaptor(const dynamic_buffer_adaptor &) = default;

    dynamic_buffer_adaptor &operator=(const dynamic_buffer_adaptor &) = default;

    dynamic_buffer_adaptor(dynamic_buffer_adaptor &&) noexcept = default;

    dynamic_buffer_adaptor &operator=(dynamic_buffer_adaptor &&) noexcept = default;

public:
    using const_buffers_type = net::const_buffer;

    using mutable_buffers_type = net::mutable_buffer;

    [[nodiscard]] net::mutable_buffer data(size_t pos, size_t n);

    [[nodiscard]] net::const_buffer data(size_t pos, size_t n) const;

    void grow(size_t n);

    [[nodiscard]] size_t size() const;

    void shrink(size_t n);

    [[nodiscard]] size_t capacity() const;

    void consume(size_t n);

    [[nodiscard]] size_t max_size() const;

private:
    pro::proxy<detail::facade_dynamic_buffer> dynamic_buffer_;
};

template<is_dynamic_buffer DynamicBuffer>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(DynamicBuffer *dynamic_buffer)
    : dynamic_buffer_(dynamic_buffer)
{}

template<is_dynamic_buffer DynamicBuffer>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(DynamicBuffer &dynamic_buffer)
    : dynamic_buffer_(std::addressof(dynamic_buffer))
{}

template<is_small_dynamic_buffer DynamicBuffer>
    requires(!std::is_reference_v<DynamicBuffer>)
dynamic_buffer_adaptor::dynamic_buffer_adaptor(DynamicBuffer &&dynamic_buffer)
    : dynamic_buffer_(pro::make_proxy<detail::facade_dynamic_buffer>(std::move(dynamic_buffer)))
{}

template<typename Elem, typename Allocator>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(std::vector<Elem, Allocator> &dynamic_buffer)
    : dynamic_buffer_(pro::make_proxy<detail::facade_dynamic_buffer>(net::dynamic_buffer(dynamic_buffer)))
{}

template<typename Elem, typename Traits, typename Allocator>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(std::basic_string<Elem, Traits, Allocator> &dynamic_buffer)
    : dynamic_buffer_(pro::make_proxy<detail::facade_dynamic_buffer>(net::dynamic_buffer(dynamic_buffer)))
{}

static_assert(net::is_dynamic_buffer_v2<dynamic_buffer_adaptor>::value);
} // namespace mrpc

#endif