//
// Created by jay on 14/10/25.
//

#ifndef REACTORPUBLISHER_H
#define REACTORPUBLISHER_H

#include "Reactor.h"

#include "helloworld.pb.h"

#include <grpcpp/support/server_callback.h>

#include <mutex>
#include <list>

class ReactorPublisher {
public:
    ReactorPublisher();

    Reactor *AddSubscriber();

    void SendMessage(proto::HelloReply &&message);

    void SendMessageAndFinish(proto::HelloReply &&message, grpc::Status status);

private:
    void CleanUpCancelledReactors();

    std::mutex m_reactorListMutex;
    std::list<Reactor *> m_reactorList;
};


#endif //REACTORPUBLISHER_H
