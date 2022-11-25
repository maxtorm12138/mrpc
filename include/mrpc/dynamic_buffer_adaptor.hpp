#ifndef MRPC_DYNAMIC_BUFFER_HPP
#define MRPC_DYNAMIC_BUFFER_HPP

#include <boost/asio/buffer.hpp>

#include <proxy.h>

#include <mrpc/detail/dynamic_buffer_adaptor.hpp>

namespace mrpc {
namespace net = boost::asio;
using namespace std::placeholders;

class dynamic_buffer_adaptor
{
public:
    template<typename DynamicBuffer>
        requires net::is_dynamic_buffer_v2<std::decay_t<DynamicBuffer>>::value
    dynamic_buffer_adaptor(DynamicBuffer *dynamic_buffer);

    template<typename DynamicBuffer>
        requires net::is_dynamic_buffer_v2<std::decay_t<DynamicBuffer>>::value
    dynamic_buffer_adaptor(DynamicBuffer &dynamic_buffer);

    template<typename DynamicBuffer>
        requires net::is_dynamic_buffer_v2<std::decay_t<DynamicBuffer>>::value && pro::proxiable<pro::details::sbo_ptr<std::decay_t<DynamicBuffer>>, detail::facade_dynamic_buffer>
    dynamic_buffer_adaptor(DynamicBuffer &&dynamic_buffer);

    template<typename Elem, typename Allocator>
        requires(sizeof(Elem) == 1) && pro::proxiable<pro::details::sbo_ptr<std::vector<Elem, Allocator>>, detail::facade_dynamic_buffer>
    dynamic_buffer_adaptor(std::vector<Elem, Allocator> &dynamic_buffer);

    template<typename Elem, typename Traits, typename Allocator>
        requires(sizeof(Elem) == 1) && pro::proxiable<pro::details::sbo_ptr<net::dynamic_string_buffer<Elem, Traits, Allocator>>, detail::facade_dynamic_buffer>
    dynamic_buffer_adaptor(std::basic_string<Elem, Traits, Allocator> &dynamic_buffer);

    dynamic_buffer_adaptor(std::nullptr_t);

    dynamic_buffer_adaptor(std::nullopt_t);

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

    [[nodiscard]] size_t capacity() const;

    void consume(size_t n);

private:
    pro::proxy<detail::facade_dynamic_buffer> dynamic_buffer_;
};

template<typename DynamicBuffer>
    requires net::is_dynamic_buffer_v2<std::decay_t<DynamicBuffer>>::value
dynamic_buffer_adaptor::dynamic_buffer_adaptor(DynamicBuffer *dynamic_buffer)
    : dynamic_buffer_(dynamic_buffer)
{}

template<typename DynamicBuffer>
    requires net::is_dynamic_buffer_v2<std::decay_t<DynamicBuffer>>::value
dynamic_buffer_adaptor::dynamic_buffer_adaptor(DynamicBuffer &dynamic_buffer)
    : dynamic_buffer_(std::addressof(dynamic_buffer))
{}

template<typename DynamicBuffer>
    requires net::is_dynamic_buffer_v2<std::decay_t<DynamicBuffer>>::value && pro::proxiable<pro::details::sbo_ptr<std::decay_t<DynamicBuffer>>, detail::facade_dynamic_buffer>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(DynamicBuffer &&dynamic_buffer)
    : dynamic_buffer_(pro::make_proxy<detail::facade_dynamic_buffer>(std::forward<std::decay_t<DynamicBuffer>>(dynamic_buffer)))
{}

template<typename Elem, typename Allocator>
    requires(sizeof(Elem) == 1) && pro::proxiable<pro::details::sbo_ptr<std::vector<Elem, Allocator>>, detail::facade_dynamic_buffer>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(std::vector<Elem, Allocator> &dynamic_buffer)
    : dynamic_buffer_(pro::make_proxy<detail::facade_dynamic_buffer>(net::dynamic_buffer(dynamic_buffer)))
{}

template<typename Elem, typename Traits, typename Allocator>
    requires(sizeof(Elem) == 1) && pro::proxiable<pro::details::sbo_ptr<net::dynamic_string_buffer<Elem, Traits, Allocator>>, detail::facade_dynamic_buffer>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(std::basic_string<Elem, Traits, Allocator> &dynamic_buffer)
    : dynamic_buffer_(pro::make_proxy<detail::facade_dynamic_buffer>(net::dynamic_buffer(dynamic_buffer)))
{}

} // namespace mrpc

#endif