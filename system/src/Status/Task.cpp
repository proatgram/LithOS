#include "Task.hpp"

using namespace Status;

Task::Task(Task::Private, const std::string &name, const std::string &description) :
    m_name(name), m_description(description)
{
    
}

auto Task::GetOrCreate(const std::string &name, const std::string &descrioption) -> std::shared_ptr<Task> {
    return std::make_shared<Task>(Private(), name, descrioption);
}

auto Task::SetName(const std::string &name) -> std::shared_ptr<Task> {
    m_name = name;

    return shared_from_this();
}

auto Task::GetName() const -> std::string {
    return m_name;
}

auto Task::SetDescription(const std::string &description) -> std::shared_ptr<Task> {
    m_description = description;

    return shared_from_this();
}

auto Task::GetDescription() const -> std::string {
    return m_description;
}

auto Task::TickProgress(float amount) -> void {
    m_progress += amount;

    m_callbackFunction(m_progress);
}

auto Task::SetProgress(float progress) -> void {
    m_progress = progress;

    m_callbackFunction(m_progress);
}

auto Task::Finish() -> void {
    SetProgress(100.0f);
    m_callbackFunction(m_progress);
    
}

auto Task::AddProgressCallback(const CallbackFunction_t callbackFunction) -> std::shared_ptr<Task> {
    m_callbackFunction = callbackFunction;
    m_callbackFunction(m_progress);

    return shared_from_this();
}
