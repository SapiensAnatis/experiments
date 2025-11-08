//
// Created by jay on 07/11/25.
//

#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <format>
#include <iostream>
#include <string>

class Logger {
public:
    Logger() { Log("Creating Logger"); }

    ~Logger() { Log("Destroying Logger"); }

    void Log(const std::string &message) {
        std::cout << std::format("[{0:%F}T{0:%X}Z] ", std::chrono::system_clock::now()) << message << std::endl;
    }
};

#endif // LOGGER_H
