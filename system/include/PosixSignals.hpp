#pragma once

namespace POSIXSignals {
    struct Signal {
        static auto InitHandlers() -> void;
        inline Signal(int code) : Code(code) {};
        int Code{};
    };

    struct SigInt : public Signal {
        inline SigInt() : Signal(2) {}
    };
}  // namespace POSIXSignals
