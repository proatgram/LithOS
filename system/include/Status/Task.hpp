#pragma once

#include <memory>
#include <string>
#include <functional>

    class Task : public std::enable_shared_from_this<Task> {
        struct Private{ inline explicit Private() {} };
        public:
            using CallbackFunction_t = std::function<void(float)>;
            static auto GetOrCreate(const std::string &name, const std::string &description = "") -> std::shared_ptr<Task>;

            Task(Task::Private, const std::string &name, const std::string &description = "");

            auto SetName(const std::string &name) -> std::shared_ptr<Task>;
            auto GetName() const -> std::string;

            auto SetDescription(const std::string &description) -> std::shared_ptr<Task>;
            auto GetDescription() const -> std::string;

            auto TickProgress(float amount = 1.0f) -> void;
            auto SetProgress(float progress) -> void;
            auto GetProgress() const -> float;

            auto Finish() -> void;
            auto IsFinished() const -> bool;

            // Used by `Status` to control the actual progress bars backed by `indicators`.
            // Shouldn't be used unless explicitely intended.
            auto AddProgressCallback(const CallbackFunction_t callbackFunction) -> std::shared_ptr<Task>;


        private:
            float m_progress{};

            CallbackFunction_t m_callbackFunction;

            std::string m_name;
            std::string m_description;
    };
