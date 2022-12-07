#include <mrpc/service.hpp>

namespace mrpc {
net::awaitable<sys::error_code> service::invoke(const Context &context, net::const_buffer request, dynamic_buffer_adaptor response)
{
    auto it_method = methods_.find(context.cmd_id());
    if (it_method == methods_.end())
    {
        co_return rpc_error::method_not_implement;
    }

    co_return co_await std::invoke(*it_method->second, context, request, response);
}

} // namespace mrpc