#ifndef MRPC_DETAIL_DYNAMIC_BUFFER_HPP
#define MRPC_DETAIL_DYNAMIC_BUFFER_HPP

#include <boost/asio/buffer.hpp>
namespace mrpc::detail {
namespace net = boost::asio;

class dynamic_buffer
{
public:
    template<typename Elem, typename Allocator>
    dynamic_buffer(net::dynamic_vector_buffer<Elem, Allocator> &real_dynamic_buffer);

    template<typename Elem, typename Traits, typename Allocator>
    dynamic_buffer(net::dynamic_string_buffer<Elem, Traits, Allocator> &real_dynamic_buffer);

    net::mutable_buffer prepare(size_t n);

    void commit(size_t n);

private:
    std::function<net::mutable_buffer(size_t)> fn_prepare_;
    std::function<void(size_t)> fn_commit_;
};

template<typename Elem, typename Allocator>
dynamic_buffer::dynamic_buffer(net::dynamic_vector_buffer<Elem, Allocator> &real_dynamic_buffer)
    : fn_prepare_(std::bind(&net::dynamic_vector_buffer<Elem, Allocator>::prepare, &real_dynamic_buffer, std::placeholders::_1))
    , fn_commit_(std::bind(&net::dynamic_vector_buffer<Elem, Allocator>::commit, &real_dynamic_buffer, std::placeholders::_1))
{}

} // namespace mrpc::detail

#endif