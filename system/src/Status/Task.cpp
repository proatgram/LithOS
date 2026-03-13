#include "Task.hpp"

Task::Task(Task::Private, const std::string &name, const std::string &description) :
    m_name(name), m_description(description)
{
    
}

auto Task::GetOrCreate(const std::string &name, const std::string &description) -> std::shared_ptr<Task> {
    for (const std::shared_ptr<Task> &task : s_instances) {
        if (task->GetName() == name) {
            return task;
        }
    }

    std::shared_ptr<Task> task = std::make_shared<Task>(Private(), name, description);
    s_instances.insert(task);

    return task;
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
    m_descriptionCallbackFunction(m_description);

    return shared_from_this();
}

auto Task::GetDescription() const -> std::string {
    return m_description;
}

auto Task::TickProgress(float amount) -> void {
    if (IsFinished()) {
        return;
    }

    m_progress += amount;

    m_progressCallbackFunction(m_progress);
}

auto Task::SetProgress(float progress) -> void {
    if (IsFinished()) {
        return;
    }

    m_progress = progress;

    m_progressCallbackFunction(m_progress);
}

auto Task::Finish() -> std::shared_ptr<Task> {
    SetProgress(100.0f);

    return shared_from_this();
}

auto Task::IsFinished() const -> bool {
    return m_progress >= 100.0f;
}

auto Task::AddProgressCallback(const ProgressCallbackFunction_t callbackFunction) -> std::shared_ptr<Task> {
    m_progressCallbackFunction = callbackFunction;

    return shared_from_this();
}

auto Task::AddDescriptionCallback(const DescriptionCallbackFunction_t callbackFunction) -> std::shared_ptr<Task> {
    m_descriptionCallbackFunction = callbackFunction;

    return shared_from_this();
}
