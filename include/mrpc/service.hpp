#ifndef MRPC_SERVICE_H
#define MRPC_SERVICE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

// std
#include <stdexcept>
#include <unordered_map>
#include <format>

// mrpc
#include <mrpc/method.hpp>
#include <mrpc.pb.h>

// spdlog
#include <spdlog/spdlog.h>

namespace mrpc {

class service : public boost::noncopyable
{
public:
    template<is_method Method, typename Implement>
    service &implement(Implement implement);

    net::awaitable<sys::error_code> invoke(const Context &context, net::const_buffer request, dynamic_buffer_adaptor response);

private:
    std::unordered_map<uint8_t, std::unique_ptr<abstract_method>> methods_;
};

template<is_method Method, typename Implement>
service &service::implement(Implement implement)
{
    uint64_t command_id = Method::descriptor()->options().GetExtension(cmd_id);
    auto [iter, exists] = methods_.emplace(command_id, std::make_unique<abstract_method>(new method<Method, Implement>(std::move(implement))));
    if (exists)
    {
        throw std::runtime_error(fmt::format("{} already implemented", command_id));
    }

    return *this;
}

} // namespace mrpc

#endif