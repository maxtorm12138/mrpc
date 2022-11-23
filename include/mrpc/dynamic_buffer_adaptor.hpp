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
    template<typename Elem, typename Allocator>
    dynamic_buffer_adaptor(net::dynamic_vector_buffer<Elem, Allocator> real_dynamic_buffer);

    template<typename Elem, typename Traits, typename Allocator>
    dynamic_buffer_adaptor(net::dynamic_string_buffer<Elem, Traits, Allocator> real_dynamic_buffer);

public:
    net::mutable_buffer data(size_t pos, size_t n);

    net::const_buffer data(size_t pos, size_t n) const;

    void grow(size_t n);

    size_t size() const;

private:
    std::any holder_;
    std::function<net::mutable_buffer(size_t, size_t)> data_;
    std::function<net::const_buffer(size_t, size_t)> const_data_;
    std::function<void(size_t)> grow_;
    std::function<size_t()> size_;
};

template<typename Elem, typename Allocator>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(net::dynamic_vector_buffer<Elem, Allocator> real_dynamic_buffer)
    : holder_(std::move(real_dynamic_buffer))
{
    static_assert(sizeof(Elem) == 1, "Elem size == 1 is requred");
    data_ = [this](size_t pos, size_t n) { return std::any_cast<net::dynamic_vector_buffer<Elem, Allocator>>(&holder_)->data(pos, n); };
    const_data_ = [this](size_t pos, size_t n) { return std::any_cast<const net::dynamic_vector_buffer<Elem, Allocator>>(&holder_)->data(pos, n); };
    grow_ = [this](size_t n) { std::any_cast<net::dynamic_vector_buffer<Elem, Allocator>>(&holder_)->grow(n); };
    size_ = [this]() { return std::any_cast<net::dynamic_vector_buffer<Elem, Allocator>>(&holder_)->size(); };
}

template<typename Elem, typename Traits, typename Allocator>
dynamic_buffer_adaptor::dynamic_buffer_adaptor(net::dynamic_string_buffer<Elem, Traits, Allocator> real_dynamic_buffer)
    : holder_(std::move(real_dynamic_buffer))
{
    static_assert(sizeof(Elem) == 1, "Elem size == 1 is requred");
    data_ = [this](size_t pos, size_t n) { return std::any_cast<net::dynamic_string_buffer<Elem, Traits, Allocator>>(&holder_)->data(pos, n); };
    const_data_ = [this](size_t pos, size_t n) { return std::any_cast<const net::dynamic_string_buffer<Elem, Traits, Allocator>>(&holder_)->data(pos, n); };
    grow_ = [this](size_t n) { std::any_cast<net::dynamic_string_buffer<Elem, Traits, Allocator>>(&holder_)->grow(n); };
    size_ = [this]() { return std::any_cast<net::dynamic_string_buffer<Elem, Traits, Allocator>>(&holder_)->size(); };
}
} // namespace mrpc

#endif