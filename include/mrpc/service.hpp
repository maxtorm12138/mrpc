#ifndef MRPC_SERVICE_H
#define MRPC_SERVICE_H

#include <unordered_map>
#include <format>

// mrpc
#include <mrpc/method.hpp>
#include <mrpc.pb.h>

namespace mrpc {

class service : public boost::noncopyable
{
public:
    service();

    template<is_method Method, typename Implement>
    service &implement(Implement implement);

private:
    std::unordered_map<uint8_t, std::unique_ptr<abstract_method>> methods_;
};

template<is_method Method, typename Implement>
service &service::implement(Implement implement)
{
    uint64_t command_id = Method::descriptor()->options().GetExtension(cmd_id);
    auto [iter, exists] = methods_.emplace(command_id, std::make_unique<abstract_method>(new method<Method, Implement>(std::move(implement))));
    return *this;
}

} // namespace mrpc

#endif