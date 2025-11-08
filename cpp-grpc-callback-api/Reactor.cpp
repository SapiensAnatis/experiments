//
// Created by jay on 14/10/25.
//

#include "Reactor.h"


void Reactor::OnWriteDone(bool cond) {
    std::cout << "Reactor " << this << ":  OnWriteDone\n";

    assert(m_writeLatch != nullptr);
    m_writeLatch->count_down();
    m_writeLatch = nullptr;

    m_writeInProgress.store(false);
    m_writeInProgress.notify_one();
}

void Reactor::OnCancel() {
    std::cout << "Reactor " << this << ":  OnCancel\n";
    m_cancelled.store(true);
}

void Reactor::OnDone() {
    std::cout << "Reactor " << this << ":  OnDone\n";

    // StartWriteAndFinish may not call OnWriteDone -- set the latch if it exists.
    if (m_writeLatch != nullptr) {
        m_writeLatch->count_down();
    }

    delete this;
}

void Reactor::ExternalStartWrite(const proto::HelloReply *message, std::latch *latch) {
    std::cout << "Reactor " << this << ":  ExternalStartWrite\n";

    m_writeInProgress.wait(true);

    m_writeInProgress.store(true);

    assert(m_writeLatch == nullptr);
    m_writeLatch = latch;

    StartWrite(message);
}

void Reactor::ExternalStartWriteAndFinish(const proto::HelloReply *message, const grpc::WriteOptions options,
                                          grpc::Status status, std::latch *latch) {
    std::cout << "Reactor " << this << ":  ExternalStartWriteAndFinish\n";

    m_writeInProgress.wait(true);

    m_writeInProgress.store(true);

    assert(m_writeLatch == nullptr);
    m_writeLatch = latch;

    StartWriteAndFinish(message, options, status);
}

bool Reactor::IsCancelled() const {
    return m_cancelled.load();
}
