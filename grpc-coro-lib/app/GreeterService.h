//
// Created by jay on 04/03/2026.
//

#ifndef GRPC_COROUTINE_PRACTICE_GREETERSERVICE_H
#define GRPC_COROUTINE_PRACTICE_GREETERSERVICE_H

#include "helloworld.grpc.pb.h"

namespace app {
class GreeterService : public proto::Greeter::CallbackService {
  public:
    grpc::ServerWriteReactor<proto::HelloReply> *
    SubscribeToHellos(grpc::CallbackServerContext *, const google::protobuf::Empty *) override;
};

} // namespace app

#endif // GRPC_COROUTINE_PRACTICE_GREETERSERVICE_H