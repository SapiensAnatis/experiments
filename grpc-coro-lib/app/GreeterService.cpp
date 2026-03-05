//
// Created by jay on 04/03/2026.
//

#include "GreeterService.h"

#include "grpc_coro_lib/start_coro_rpc.h"

#include <grpc_coro_lib/coro_server_write_reactor.h>
#include <grpc_coro_lib/grpc_task.h>

#include <print>

namespace app {

grpc_coro_lib::GrpcTask SubscribeToHellosCoro(
    grpc::CallbackServerContext * /*callback_server_context*/,
    const google::protobuf::Empty * /*empty*/,
    std::unique_ptr<grpc_coro_lib::ServerWriteReactor<proto::HelloReply>> reactor) {
    for (int i = 0; i < 5; ++i) {
        proto::HelloReply reply;
        reply.set_message(std::format("Hello #{}", i));

        const bool ok = co_await reactor->Write(reply);

        if (!ok) {
            std::println(stderr, "Write #{} failed!", i);
        }
    }

    co_await reactor->Finish(grpc::Status::OK);
}

grpc::ServerWriteReactor<proto::HelloReply> *
GreeterService::SubscribeToHellos(grpc::CallbackServerContext *callback_server_context,
                                  const google::protobuf::Empty *empty) {
    auto reactor = grpc_coro_lib::StartServerStreamingCoroRpc(callback_server_context, empty,
                                                              SubscribeToHellosCoro);

    return reactor;
}

} // namespace app
