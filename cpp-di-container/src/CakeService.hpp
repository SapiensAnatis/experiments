//
// Created by jay on 07/11/25.
//

#ifndef CAKESERVICE_H
#define CAKESERVICE_H

#include "CakeProtocol.hpp"
#include "Logger.hpp"

#include <memory>


class CakeService {
public:
    static std::unique_ptr<CakeService> Create(Logger *logger, CakeProtocol *protocol) {
        return std::unique_ptr<CakeService>(new CakeService(logger, protocol));
    }

    void DoThing() const {
        m_logger->Log("CakeService: Instructing CakeProtocol to do the thing");
        m_protocol->DoThing();
    }

    ~CakeService() { m_logger->Log("Destroying CakeService"); }

private:
    explicit CakeService(Logger *logger, CakeProtocol *protocol) : m_logger(logger), m_protocol(protocol) {
        m_logger->Log("Creating CakeService");
    }

    Logger *m_logger{nullptr};
    CakeProtocol *m_protocol{nullptr};
};

#endif // CAKESERVICE_H
