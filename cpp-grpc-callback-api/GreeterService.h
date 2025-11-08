//
// Created by jay on 14/10/25.
//

#ifndef GREETERSERVICE_H
#define GREETERSERVICE_H

#include "ReactorPublisher.h"

#include "helloworld.grpc.pb.h"

#include <thread>
#include <atomic>

class GreeterService : public proto::Greeter::CallbackService {
public:
    grpc::ServerWriteReactor<proto::HelloReply> *SubscribeToHellos(grpc::CallbackServerContext *context,
                                                                   const google::protobuf::Empty *request) override;

private:
    ReactorPublisher m_publisher;
    std::jthread m_thread;
    std::atomic<bool> m_threadActive{false};
};


#endif //GREETERSERVICE_H
