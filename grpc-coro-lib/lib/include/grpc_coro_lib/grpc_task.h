//
// Created by jay on 05/03/2026.
//

#ifndef GRPC_COROUTINE_PRACTICE_GRPC_TASK_H
#define GRPC_COROUTINE_PRACTICE_GRPC_TASK_H

#include <grpc_coro_lib/coro_server_write_reactor.h>
#include <grpc_coro_lib/grpc_final_awaiter.h>

#include <coroutine>
#include <exception>

namespace grpc_coro_lib {

struct GrpcTask {
    struct promise_type {
        GrpcTask get_return_object() {
            return GrpcTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // Always suspend: we want to set the reactor before we start the coroutine to avoid a race
        // if the RPC method immediately throws an exception
        std::suspend_always initial_suspend() { return {}; }

        auto final_suspend() noexcept { return internal::GrpcFinalAwaiter<promise_type>{}; }

        void return_void() {}

        void unhandled_exception() { exception_ = std::current_exception(); }

        std::exception_ptr exception_;
        std::function<void(grpc::Status)> finish_;
    };

    std::coroutine_handle<promise_type> handle_;

    explicit GrpcTask(const std::coroutine_handle<promise_type> h) : handle_(h) {}
};

} // namespace grpc_coro_lib

#endif // GRPC_COROUTINE_PRACTICE_GRPC_TASK_H
