//
// Created by jay on 05/03/2026.
//

#ifndef GRPC_COROUTINE_PRACTICE_CORO_SERVER_WRITE_REACTOR_H
#define GRPC_COROUTINE_PRACTICE_CORO_SERVER_WRITE_REACTOR_H

#include <coroutine>
#include <mutex>

#include <grpcpp/support/server_callback.h>

namespace grpc_coro_lib {

template <typename Message> class ServerWriteReactor;

namespace internal {

template <typename Message>
class CoroHandleServerWriteReactor : public grpc::ServerWriteReactor<Message> {
  public:
    void OnDone() override {
        std::unique_lock lock(mutex_);

        // We may not have a finish handle if we are finishing due to a coroutine exception
        if (finish_handle_) {
            const auto handle = std::exchange(finish_handle_, std::coroutine_handle<>{});
            lock.unlock();
            handle.resume();
        }
    }

    void OnWriteDone(const bool ok) override {
        std::unique_lock lock(mutex_);

        assert(current_write_ok_);
        *current_write_ok_ = ok;
        current_write_ok_ = nullptr;

        assert(write_handle_);
        const auto handle = std::exchange(write_handle_, std::coroutine_handle<>{});
        lock.unlock();
        handle.resume();
    };

    void StartWriteWithHandle(Message *message, std::coroutine_handle<> handle,
                              bool *write_result) {
        {
            std::lock_guard lock(mutex_);
            write_handle_ = handle;
            current_write_ok_ = write_result;
        }

        this->StartWrite(message);
    }

    void FinishWithHandle(std::coroutine_handle<> handle, grpc::Status status) {
        {
            std::lock_guard lock(mutex_);
            finish_handle_ = handle;
        }

        this->Finish(status);
    }

  private:
    std::mutex mutex_;
    std::coroutine_handle<> write_handle_;
    std::coroutine_handle<> finish_handle_;
    bool *current_write_ok_ = nullptr;
};

} // namespace internal

template <typename Message> class ServerWriteReactor {
  private:
    using CoroHandleReactor = internal::CoroHandleServerWriteReactor<Message>;

    struct WriteAwaitable {
        explicit WriteAwaitable(CoroHandleReactor *reactor, Message message)
            : reactor_(reactor), message_(std::move(message)) {}

        bool await_ready() const noexcept { return false; }

        void await_suspend(std::coroutine_handle<> handle) {
            reactor_->StartWriteWithHandle(&message_, handle, &write_ok_);
        }

        bool await_resume() const noexcept { return write_ok_; }

        CoroHandleReactor *reactor_;
        Message message_;
        bool write_ok_ = false;
    };

    struct FinishAwaitable {
        explicit FinishAwaitable(CoroHandleReactor *reactor, grpc::Status status)
            : reactor_(reactor), status_(std::move(status)) {}

        bool await_ready() const noexcept { return false; }

        void await_suspend(std::coroutine_handle<> handle) {
            reactor_->FinishWithHandle(handle, status_);
        }

        void await_resume() const noexcept {}

        CoroHandleReactor *reactor_;
        grpc::Status status_;
    };

  public:
    explicit ServerWriteReactor(std::unique_ptr<CoroHandleReactor> inner_reactor)
        : inner_reactor_(std::move(inner_reactor)) {}

    [[nodiscard]] WriteAwaitable Write(Message message) {
        return WriteAwaitable(inner_reactor_.get(), std::move(message));
    }

    [[nodiscard]] FinishAwaitable Finish(grpc::Status status) {
        return FinishAwaitable(inner_reactor_.get(), std::move(status));
    }

  private:
    std::unique_ptr<CoroHandleReactor> inner_reactor_;
};

} // namespace grpc_coro_lib

#endif // GRPC_COROUTINE_PRACTICE_CORO_SERVER_WRITE_REACTOR_H
