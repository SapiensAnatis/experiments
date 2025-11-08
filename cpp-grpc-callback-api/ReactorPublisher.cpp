#include "ReactorPublisher.h"

ReactorPublisher::ReactorPublisher() = default;

Reactor *ReactorPublisher::AddSubscriber() {
    std::unique_lock lock{m_reactorListMutex};

    const auto reactor = new Reactor();
    m_reactorList.push_back(reactor);

    return reactor;
}

void ReactorPublisher::SendMessage(proto::HelloReply &&message) {
    std::unique_lock lock{m_reactorListMutex};

    CleanUpCancelledReactors();

    if (m_reactorList.empty()) {
        return;
    }

    std::latch writeLatch{static_cast<std::ptrdiff_t>(m_reactorList.size())};

    for (Reactor *reactor: m_reactorList) {
        reactor->ExternalStartWrite(&message, &writeLatch);
    }

    lock.unlock();

    writeLatch.wait();
}

void ReactorPublisher::SendMessageAndFinish(proto::HelloReply &&message, grpc::Status status) {
    grpc::WriteOptions opts;
    opts.set_last_message();

    std::lock_guard lock{m_reactorListMutex};

    CleanUpCancelledReactors();

    if (m_reactorList.empty()) {
        return;
    }

    std::latch writeLatch{static_cast<std::ptrdiff_t>(m_reactorList.size())};

    for (Reactor *reactor: m_reactorList) {
        if (reactor->IsCancelled()) {
            reactor->Finish(grpc::Status::CANCELLED);
            // We will clear at the end anyway
            continue;
        }

        reactor->ExternalStartWriteAndFinish(&message, opts, status, &writeLatch);
    }

    writeLatch.wait();

    m_reactorList.clear();
}

void ReactorPublisher::CleanUpCancelledReactors() {
    for (auto iter = m_reactorList.begin(); iter != m_reactorList.end();) {
        if (Reactor *reactor = *iter; reactor->IsCancelled()) {
            reactor->Finish(grpc::Status::CANCELLED);
            iter = m_reactorList.erase(iter);
        } else {
            ++iter; // Only increment if not erased; erase invalidates iter
        }
    }
}
