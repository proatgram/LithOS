#include "Status.hpp"

using namespace indicators;

auto Status::Status::GetOrCreate() -> std::shared_ptr<Status> {
    if (!s_instance) {
        s_instance = std::make_shared<Status>(Private());
    }

    return s_instance;
}

// We need to disable and re-enable the cursor during progress output
// or else it will look weird
Status::Status::Status(Status::Status::Private) {
    show_console_cursor(false);
}

Status::Status::~Status() {
    show_console_cursor(true);
}

auto Status::Status::AddTask(const std::shared_ptr<Task> &task) -> std::shared_ptr<Status> {
    m_tasks.push_back(task);
    m_progressBars.emplace_back(
        option::PostfixText{task->GetDescription()},
        option::ShowElapsedTime{true},
        option::ShowPercentage{true},
        option::BarWidth{50},
        option::Start{"["},
        option::Fill{"-"},
        option::Lead{">"},
        option::Remainder{" "},
        option::End{"]"},
        option::ForegroundColor{Color::white},
        option::ShowRemainingTime{true},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
    );
    ProgressBar &bar = m_progressBars.back();
    m_dynamicProgress.push_back(bar);
    
    task->AddProgressCallback([this, &bar, task](float progress) -> void {
        bar.set_progress(progress);
        m_dynamicProgress.print_progress();
        if (bar.is_completed()) {
            // Reset callback so that it's not accessing an invalid reference
            // just in case it gets used again when `Status` gets reset
            task->AddProgressCallback([](float progress) -> void {});
        }
    });

    return shared_from_this();
}

auto Status::Status::GetTasks() -> std::vector<std::shared_ptr<Task>> {
    return m_tasks;
}

auto Status::Status::Finish() -> void {
    s_instance.reset();
}
