#ifndef MRPC_BATCH_RUN_HPP
#define MRPC_BATCH_RUN_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/noncopyable.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/cancellation_condition.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/any_completion_handler.hpp>

namespace mrpc {
namespace net = boost::asio;

template<typename T>
class batch_run : public boost::noncopyable
{
public:
    explicit batch_run(const net::any_io_executor &executor);

    template<typename CompletionToken>
    auto async_wait(CompletionToken &&token);

private:
    const net::any_io_executor &executor_;
    std::shared_ptr<>
};

template<typename T>
batch_run<T>::batch_run(const net::any_io_executor &executor)
    : executor_(executor)
{}

} // namespace mrpc

#endif
