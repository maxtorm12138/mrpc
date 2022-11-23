#ifndef MRPC_METHOD_H
#define MRPC_METHOD_H

// boost
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/noncopyable.hpp>

// mrpc
#include <mrpc/dynamic_buffer_adaptor.hpp>
#include <mrpc/error_code.hpp>

// protobuf
#include <google/protobuf/message.h>

namespace mrpc {
namespace net = boost::asio;
namespace sys = boost::system;

class stub;

struct context
{
    std::weak_ptr<stub> stub;
    std::string trace;
};

class abstract_method : public boost::noncopyable
{
public:
    virtual net::awaitable<sys::error_code> operator()(context ctx, net::const_buffer request, dynamic_buffer_adaptor response) = 0;
    virtual ~abstract_method() = default;
};

template<typename Method>
concept is_method = requires {
                        requires std::derived_from<Method, google::protobuf::Message>;
                        requires std::derived_from<typename Method::Request, google::protobuf::Message>;
                        requires std::derived_from<typename Method::Response, google::protobuf::Message>;
                    };

template<typename Method, typename Implement>
concept is_method_implement = requires {
                                  requires is_method<Method>;
                                  requires std::is_nothrow_move_constructible_v<Implement>;
                                  requires std::is_nothrow_move_assignable_v<Implement>;
                                  requires std::same_as<std::invoke_result_t<Implement, context, const typename Method::Request &>, net::awaitable<typename Method::Response>>;
                              };

template<is_method Method, typename Implement>
    requires is_method_implement<Method, Implement>
class method final : public abstract_method
{
public:
    using request_type = typename Method::Request;
    using response_type = typename Method::Response;

    explicit method(Implement implement);

    net::awaitable<sys::error_code> operator()(context ctx, net::const_buffer request, dynamic_buffer_adaptor response) override;

private:
    Implement implement_;
};

template<is_method Method, typename Implement>
    requires is_method_implement<Method, Implement>
method<Method, Implement>::method(Implement implement)
    : implement_(std::move(implement))
{}

template<is_method Method, typename Implement>
    requires is_method_implement<Method, Implement>
net::awaitable<sys::error_code> method<Method, Implement>::operator()(context ctx, net::const_buffer request, dynamic_buffer_adaptor response)
{
    request_type req;

    if (!req.ParseFromArray(request.data(), request.size()))
    {
        co_return rpc_error::proto_parse_fail;
    }

    response_type resp = co_await std::invoke(implement_, ctx, std::cref(req));

    auto pos = response.size();
    auto size = resp.ByteSizeLong();

    response.grow(size);
    auto buffer = response.data(pos, size);

    if (!resp.SerializeToArray(buffer.data(), buffer.size()))
    {
        co_return rpc_error::proto_serialize_fail;
    }

    co_return rpc_error::success;
}

} // namespace mrpc

#endif