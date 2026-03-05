//
// Created by jay on 05/03/2026.
//

#ifndef GRPC_COROUTINE_PRACTICE_START_CORO_RPC_H
#define GRPC_COROUTINE_PRACTICE_START_CORO_RPC_H

#include <grpc_coro_lib/coro_server_write_reactor.h>
#include <grpc_coro_lib/grpc_task.h>

#include <grpcpp/support/server_callback.h>

namespace grpc_coro_lib {

// GrpcTask<Response> (*coro_rpc)(grpc::ClientContext *, const Request *,
// std::unique_ptr<ServerWriteReactor<Response>>)

template <typename Response, typename Request>
grpc::ServerWriteReactor<Response> *
StartServerStreamingCoroRpc(grpc::CallbackServerContext *context, Request *request,
                            GrpcTask (*coro_rpc)(grpc::CallbackServerContext *,
                                                 std::add_const_t<Request> *,
                                                 std::unique_ptr<ServerWriteReactor<Response>>)) {
    auto *reactor = new internal::CoroHandleServerWriteReactor<Response>();

    auto coro_reactor = std::make_unique<ServerWriteReactor<Response>>(
        std::unique_ptr<internal::CoroHandleServerWriteReactor<Response>>(reactor));

    auto t = coro_rpc(context, request, std::move(coro_reactor));
    t.handle_.promise().finish_ =
        std::bind_front(&grpc::ServerWriteReactor<Response>::Finish, reactor);
    t.handle_.resume();
    return reactor;
}

} // namespace grpc_coro_lib

#endif // GRPC_COROUTINE_PRACTICE_START_CORO_RPC_H
