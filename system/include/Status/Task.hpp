#pragma once

#include <memory>
#include <string>
#include <functional>
#include <set>
#include <any>
#include <optional>

class Task : public std::enable_shared_from_this<Task> {
struct Private{ inline explicit Private() {} };
public:
    using ProgressCallbackFunction_t = std::function<void(float)>;
    using DescriptionCallbackFunction_t = std::function<void(std::string)>;

    static auto GetOrCreate(const std::string &name, const std::string &description = "") -> std::shared_ptr<Task>;

    Task(Task::Private, const std::string &name, const std::string &description = "");

    auto SetName(const std::string &name) -> std::shared_ptr<Task>;
    auto GetName() const -> std::string;

    auto SetDescription(const std::string &description) -> std::shared_ptr<Task>;
    auto GetDescription() const -> std::string;

    auto TickProgress(float amount = 1.0f) -> void;
    auto SetProgress(float progress) -> void;
    auto GetProgress() const -> float;

    auto Finish() -> std::shared_ptr<Task>;
    auto IsFinished() const -> bool;

    // Used by `Status` to control the actual progress bars backed by `indicators`.
    // Shouldn't be used unless explicitely intended.
    auto AddProgressCallback(const ProgressCallbackFunction_t callbackFunction) -> std::shared_ptr<Task>;

    auto AddDescriptionCallback(const DescriptionCallbackFunction_t callbackFunction) -> std::shared_ptr<Task>;

    template <typename T>
    requires (!(std::is_reference_v<T> && std::is_const_v<T>))
    inline auto GetContext() -> std::optional<T> {
        if (m_context.has_value() && typeid(T) == m_context.type()) {
            T val = std::any_cast<T>(m_context);
            return std::make_optional<T>(val);
        }

        return std::nullopt;
    }
    template <typename T>
    requires (!(std::is_reference_v<T> && std::is_const_v<T>))
    inline auto SetContext(const T &contextData) -> std::shared_ptr<Task> {
        m_context = std::make_any<T>(contextData);
        return shared_from_this();
    }


private:
    inline static std::set<std::shared_ptr<Task>> s_instances{};
    float m_progress{};

    ProgressCallbackFunction_t m_progressCallbackFunction;
    DescriptionCallbackFunction_t m_descriptionCallbackFunction;

    std::string m_name;
    std::string m_description;

    std::any m_context;
};
