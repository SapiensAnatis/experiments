//
// Created by jay on 05/03/2026.
//

#ifndef GRPC_COROUTINE_PRACTICE_GRPC_FINAL_AWAITER_H
#define GRPC_COROUTINE_PRACTICE_GRPC_FINAL_AWAITER_H

#include <grpcpp/support/status.h>

#include <coroutine>

namespace grpc_coro_lib::internal {

template <typename Promise> struct GrpcFinalAwaiter {
    bool await_ready() noexcept { return false; }

    void await_suspend(std::coroutine_handle<Promise> h) noexcept {
        const auto &promise = h.promise();
        if (promise.exception_) {
            promise.finish_(grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Coroutine exception"));
        }
    }

    void await_resume() noexcept {}
};

} // namespace grpc_coro_lib::internal

#endif // GRPC_COROUTINE_PRACTICE_GRPC_FINAL_AWAITER_H
