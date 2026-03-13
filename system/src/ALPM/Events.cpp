#include "Events.hpp"

#include "Event.hpp"
#include "Status.hpp"
#include "ALPM.hpp"
#include <alpm.h>

using namespace ALPM;

// This associates the archaic C-style events with our C++-based event system.
auto Events::RegisterEvents() -> void {
    // Download event
    Event::Event::RegisterCallback<DownloadEvent>([](const DownloadEvent &event) -> void {
        switch (event.Type) {
            case DownloadEvent::DownloadType::Init: {
                // TODO: Add the package download task to the progress system
                auto task = Task::GetOrCreate(event.Filename, std::format("Downloading {}", event.Filename)) ;
                alpm_download_event_init_t *init = static_cast<alpm_download_event_init_t*>(event.Context);
                task->SetContext(static_cast<bool>(init->optional));
                Status::Status::GetOrCreate()->AddTask(task);
                
                break;
            }
            case DownloadEvent::DownloadType::Progress: {
                // Don't get this Progress event confused with alpm_cb_progress, this
                // is for specifically download progress.
                float progress{};
                alpm_download_event_progress_t *prog = static_cast<alpm_download_event_progress_t*>(event.Data);
                progress = (float)prog->downloaded / (float)prog->total;
                Task::GetOrCreate(event.Filename)->SetProgress(progress);

                break;
            }
            case DownloadEvent::DownloadType::Retry: {
                Task::GetOrCreate(event.Filename)->SetDescription(std::format("Downloading {} (retrying)", event.Filename));
                break;
            }
            case DownloadEvent::DownloadType::Completed: {
                auto task = Task::GetOrCreate(event.Filename)->Finish();
                alpm_download_event_completed_t *comp = static_cast<alpm_download_event_completed_t*>(event.Data);
                // If the download is set to optional (stored in the context) then it shouldn't error out
                if (comp->result == -1 && !(task->GetContext<bool>().value_or(false))) {
                    // TODO: Error system
                    throw std::runtime_error(std::format("Failed to download file {}.", event.Filename));
                }
                break;
            }
        }
    });
    alpm_option_set_dlcb(ALPM::GetHandle(), [](void *ctx, const char *filename, alpm_download_event_type_t event, void *data) -> void {
            Event::Event::Emit<DownloadEvent>({.Context = ctx, .Filename = filename, .Type = static_cast<DownloadEvent::DownloadType>(event), .Data = data});
    }, nullptr);
}
