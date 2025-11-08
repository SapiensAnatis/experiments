//
// Created by jay on 07/11/25.
//

#ifndef SERVER_H
#define SERVER_H

#include "CakeService.hpp"
#include "Logger.hpp"

#include <memory>

class Server {
public:
    static std::unique_ptr<Server> Create(CakeService *service, Logger *logger) {
        return std::unique_ptr<Server>(new Server(service, logger));
    }

    void DoThing() const {
        m_logger->Log("Server: Instructing CakeService to do the thing");
        m_service->DoThing();
    }

    ~Server() { m_logger->Log("Destroying Server"); }

private:
    explicit Server(CakeService *service, Logger *logger) : m_service(service), m_logger(logger) {
        m_logger->Log("Creating Server");
    }

    CakeService *m_service{nullptr};
    Logger *m_logger{nullptr};
};


#endif // SERVER_H
