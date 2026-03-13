#pragma once

#include <string>

struct test;

namespace ALPM {

    struct DownloadEvent {
        enum class DownloadType {
            Init,
            Progress,
            Retry,
            Completed
        };

        void *Context;
        std::string Filename;
        DownloadType Type;
        void *Data;
    };

    class Events {
        public:
            static auto RegisterEvents() -> void;
    };
}  // namespace ALPM
