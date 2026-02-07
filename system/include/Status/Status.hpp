#pragma once

#include <vector>
#include <memory>
#include <deque>

#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>

#include "Task.hpp"

namespace Status {
    class Status : public std::enable_shared_from_this<Status> {
        struct Private { inline explicit Private() {} };
        public:
            static auto GetOrCreate() -> std::shared_ptr<Status>;

            Status(Status::Private);
            ~Status();

            auto AddTask(const std::shared_ptr<Task> &task) -> std::shared_ptr<Status>;
            auto GetTasks() -> std::vector<std::shared_ptr<Task>>;

            static auto Finish() -> void;

        private:
            inline static std::shared_ptr<Status> s_instance{};

            std::vector<std::shared_ptr<Task>> m_tasks;
            std::deque<indicators::ProgressBar> m_progressBars;
            indicators::DynamicProgress<indicators::ProgressBar> m_dynamicProgress;
    };
}  // namespace Status
