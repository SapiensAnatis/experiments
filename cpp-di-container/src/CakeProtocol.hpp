//
// Created by jay on 07/11/25.
//

#ifndef CAKEPROTOCOL_H
#define CAKEPROTOCOL_H

#include "Logger.hpp"

#include <memory>


class CakeProtocol {

public:
    static std::unique_ptr<CakeProtocol> Create(Logger *logger) {
        return std::unique_ptr<CakeProtocol>(new CakeProtocol(logger));
    }

    void DoThing() const { m_logger->Log("CakeProtocol: Doing thing"); }

    ~CakeProtocol() { m_logger->Log("Destroying CakeProtocol"); }

private:
    explicit CakeProtocol(Logger *logger) : m_logger(logger) { m_logger->Log("Creating CakeProtocol"); }

    Logger *m_logger{nullptr};
};


#endif // CAKEPROTOCOL_H
