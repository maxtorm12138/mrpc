#include <mrpc/packet_handler.hpp>

namespace mrpc {

sys::error_code abstract_packet_handler::translate(sys::error_code ec)
{
    if (ec == net::error::operation_aborted || ec == net::experimental::error::channel_cancelled)
    {
        return rpc_error::operation_canceled;
    }

    if (ec == net::error::eof || ec == net::error::broken_pipe || ec == net::error::connection_reset || ec == net::experimental::error::channel_closed)
    {
        return rpc_error::connection_closed;
    }

    if (ec)
    {
        return ec;
    }

    return rpc_error::success;
}

} // namespace mrpc