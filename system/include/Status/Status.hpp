#pragma once

#include <vector>
#include <memory>

namespace Status {
    class Status {
        public:
            
        private:
    };
}  // namespace Status

/*
    std::shared_ptr<Status> status = Status::GerOrCreate();

    std::string packageName = "libpacman-v1.2.3";

    std::shared_ptr<Task> task = Task::CreateTask(packageName);
    task->SetDescription(std::format("Downloading {}", packageName));

    status->AddTask(task, taskOptions);

    task->TickProgress(1.0);
    task->SetProgress(60.1);
    task->Finish();
*/
