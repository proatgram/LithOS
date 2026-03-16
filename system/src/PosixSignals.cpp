#include "PosixSignals.hpp"
#include "Event.hpp"

#include <csignal>
#include <iostream>

using namespace POSIXSignals;

auto Signal::InitHandlers() -> void {
    signal(SIGINT, [](int signal) -> void {
        Event::Event::Emit<SigInt>();
        std::cout << "\n\nRecieved interrupt, cleaning up..." << std::endl;
        exit(1);
    });
}
