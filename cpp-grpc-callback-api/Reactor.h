//
// Created by jay on 14/10/25.
//

#ifndef REACTOR_H
#define REACTOR_H


#include "helloworld.pb.h"

#include <grpcpp/grpcpp.h>

#include <latch>

class Reactor final : public grpc::ServerWriteReactor<proto::HelloReply> {
public:
    void OnWriteDone(bool) override;

    void OnCancel() override;

    void OnDone() override;

    void ExternalStartWrite(const proto::HelloReply *message, std::latch *latch);

    void ExternalStartWriteAndFinish(const proto::HelloReply *message, grpc::WriteOptions options,
                                     grpc::Status status,
                                     std::latch *latch);

    bool IsCancelled() const;

    std::latch *m_writeLatch{nullptr};
    std::atomic<bool> m_writeInProgress{false};
    std::atomic<bool> m_cancelled{false};
};

#endif //REACTOR_H
