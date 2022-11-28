#ifndef MRPC_STUB_H
#define MRPC_STUB_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <mrpc/packet_handler.hpp>
#include <mrpc/service.hpp>
#include <mrpc.pb.h>

#include <boost/asio/experimental/channel.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/scope_exit.hpp>
#include <boost/asio/recycling_allocator.hpp>
#include <boost/asio/any_completion_handler.hpp>

namespace mrpc {
class stub
{
private:
    template<typename T>
    using container_type = std::vector<T, net::recycling_allocator<T>>;
    using channel_type = net::experimental::channel<void(sys::error_code, container_type<uint8_t>)>;

public:
    struct options
    {
        size_t worker;
        size_t packet_buffer;
    };

public:
    template<typename Executor, typename PacketHandler>
        requires std::derived_from<PacketHandler, abstract_packet_handler>
    stub(const Executor &executor, PacketHandler packet_handler, const options &option, std::shared_ptr<service> service = nullptr);

    abstract_packet_handler &packet_handler();

    const boost::uuids::uuid &id() const;

    net::awaitable<void> async_run();

private:
    net::awaitable<void> launch(size_t launch_id);

    net::awaitable<void> input_logic(size_t launch_id);

    net::awaitable<void> output_logic(size_t launch_id);

    net::awaitable<void> worker_logic(size_t launch_id);

    net::awaitable<std::tuple<mrpc::Context, net::const_buffer>> unpack(net::const_buffer buffer);

    net::awaitable<void> pack(const mrpc::Context &context, net::const_buffer message, dynamic_buffer_adaptor buffer);

private:
    options option_;
    std::unique_ptr<abstract_packet_handler> packet_handler_;
    std::shared_ptr<service> service_;
    boost::uuids::uuid id_;
    channel_type in_;
    channel_type worker_;
    channel_type out_;
    std::unordered_map<std::string, std::optional<channel_type>> call_;
};

template<typename Executor, typename PacketHandler>
    requires std::derived_from<PacketHandler, abstract_packet_handler>
stub::stub(const Executor &executor, PacketHandler packet_handler, const options &option, std::shared_ptr<service> service)
    : option_(option)
    , packet_handler_(std::make_unique<PacketHandler>(std::move(packet_handler)))
    , service_(service)
    , id_(boost::uuids::random_generator{}())
    , in_(executor, option_.packet_buffer)
    , worker_(executor, option_.packet_buffer)
    , out_(executor, option_.packet_buffer)
{}

} // namespace mrpc
#endif