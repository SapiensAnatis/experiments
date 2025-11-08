//
// Created by jay on 14/10/25.
//

#include "GreeterService.h"

#include <thread>
#include <chrono>

grpc::ServerWriteReactor<proto::HelloReply> *GreeterService::SubscribeToHellos(grpc::CallbackServerContext *context,
                                                                               const google::protobuf::Empty *request) {
    Reactor *reactor = m_publisher.AddSubscriber();

    if (!m_threadActive.load()) {
        if (m_thread.joinable()) {
            m_thread.join();
        }

        m_thread = std::jthread([this] {
            m_threadActive.store(true);

            for (int i = 0; i < 15; i++) {
                auto message = proto::HelloReply{};
                message.set_message(std::to_string(i));

                m_publisher.SendMessage(std::move(message));

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            auto message = proto::HelloReply{};
            message.set_message("end");

            m_publisher.SendMessageAndFinish(std::move(message), grpc::Status::OK);

            m_threadActive.store(false);
        });
    }

    return reactor;
}
