#include "Events.hpp"

#include "Event.hpp"
#include "Status.hpp"
#include "ALPM.hpp"
#include "Utils.hpp"

#include <alpm.h>

#include <algorithm>
#include <cctype>

using namespace ALPM;

// This associates the archaic C-style events with our C++-based event system.
auto Events::RegisterEvents() -> void {
    // Download event
    Event::Event::RegisterCallback<DownloadEvent>([](const DownloadEvent &event) -> void {
        switch (event.Type) {
            case DownloadEvent::DownloadType::Init: {
                // TODO: Add the package download task to the progress system
                auto task = Task::GetOrCreate(event.Filename);
                alpm_download_event_init_t *init = static_cast<alpm_download_event_init_t*>(event.Context);
                if (init) {
                    task->SetContext(static_cast<bool>(init->optional));
                }
                Status::Status::GetOrCreate()->AddTask(task);
                
            }
            break;
            case DownloadEvent::DownloadType::Progress: {
                // Don't get this Progress event confused with alpm_cb_progress, this
                // is for specifically download progress.
                float progress{};
                alpm_download_event_progress_t *prog = static_cast<alpm_download_event_progress_t*>(event.Data);
                progress = ((float)prog->downloaded / (float)prog->total) * 100.0f;
                Task::GetOrCreate(event.Filename)->SetProgress(progress);

            }
            break;
            case DownloadEvent::DownloadType::Retry: {
                Task::GetOrCreate(event.Filename)->SetDescription("(Retrying)");
            }
            break;
            case DownloadEvent::DownloadType::Completed: {
                auto task = Task::GetOrCreate(event.Filename)->Finish();
                alpm_download_event_completed_t *comp = static_cast<alpm_download_event_completed_t*>(event.Data);
                // If the download is set to optional (stored in the context) then it shouldn't error out
                if (comp->result == -1 && !(task->GetContext<bool>().value_or(false))) {
                    // TODO: Error system
                    throw std::runtime_error(std::format("Failed to download file {}.", event.Filename));
                }
            }
            break;
        }
    });

    Event::Event::RegisterCallback<GenericQuestionEvent*>([](const GenericQuestionEvent *event) -> void {
        switch (event->Question) {
            case GenericQuestionEvent::QuestionType::InstallIgnorePackage: {
                const InstallIgnorePackageQuestion *question = static_cast<const InstallIgnorePackageQuestion*>(event);

                std::string choice{'\0'};
                while (choice[0] != 'n' && choice[0] != 'y' && !choice.empty()) {
                    std::cout << "Package is in ignored package list: " << question->IgnoredPackage.GetName() << std::endl;
                    std::cout << "Install anyway? [y/N] > " << std::flush;

                    std::getline(std::cin, choice);
                    std::for_each(choice.begin(), choice.end(), [](char &c) -> void {c = std::tolower(c);});


                    if (choice.size() > 1 || !(choice[0] == 'y' || choice[0] == 'n' || choice.empty())) {
                        std::cout << "Invalid answer : \"" << choice << "\"." << std::endl;
                    } else if (choice.empty() || choice[0] == 'n') {
                        *question->Answer = false;
                    } else {
                        *question->Answer = true;
                    }
                }
            }
            break;
            case GenericQuestionEvent::QuestionType::ReplacePackage: {
                const ReplacePackageQuestion *question = static_cast<const ReplacePackageQuestion*>(event);

                std::string choice{'\0'};
                while (choice[0] != 'n' && choice[0] != 'y' && !choice.empty()) {
                    std::cout << "Replace " << question->OldPackage.GetDatabase()->GetName() << '/'
                                            << question->OldPackage.GetName() << " with "
                                            << question->DatabaseOfNewPackage.GetName() << '/'
                                            << question->NewPackage.GetName() << "? [Y/n] > " << std::flush;

                    std::getline(std::cin, choice);
                    std::for_each(choice.begin(), choice.end(), [](char &c) -> void {c = std::tolower(c);});

                    if (choice.size() > 1 || !(choice[0] == 'y' || choice[0] == 'n' || choice.empty())) {
                        std::cout << "Invalid answer : \"" << choice << "\"." << std::endl;
                    } else if (choice[0] == 'n') {
                        *question->Answer = false;
                    } else {
                        *question->Answer = true;
                    }
                }
            }
            break;
            case GenericQuestionEvent::QuestionType::ConflictingPackage: {
                const ConflictingPackageQuestion *question = static_cast<const ConflictingPackageQuestion*>(event);

                const auto &[pkg1, pkg2] = question->ConflictInfo.GetPackages();

                std::string choice{'\0'};
                while (choice[0] != 'n' && choice[0] != 'y' && !choice.empty()) {
                    std::cout << pkg1.GetName() << '-' << pkg1.GetVersion() << " and "
                              << pkg2.GetName() << '-' << pkg2.GetVersion() << " are in conflict.\n"
                              << "Remove " << pkg2.GetName() << "? [y/N] > " << std::flush;

                    std::getline(std::cin, choice);
                    std::for_each(choice.begin(), choice.end(), [](char &c) -> void {c = std::tolower(c);});

                    if (choice.size() > 1 || !(choice[0] == 'y' || choice[0] == 'n' || choice.empty())) {
                        std::cout << "Invalid answer : \"" << choice << "\"." << std::endl;
                    } else if (choice.empty() || choice[0] == 'n') {
                        *question->Answer = false;
                    } else {
                        *question->Answer = true;
                    }
                }
            }
            break;
            case GenericQuestionEvent::QuestionType::CurruptedPackage: {
                const CurruptedPackageQuestion *question = static_cast<const CurruptedPackageQuestion*>(event);
                
                std::string choice{'\0'};
                while (choice[0] != 'n' && choice[0] != 'y' && !choice.empty()) {
                    std::cout << "Package at " << question->CurruptedFile.string()
                              << " is currupted. Remove? [Y/n] > " << std::flush;

                    std::getline(std::cin, choice);
                    std::for_each(choice.begin(), choice.end(), [](char &c) -> void {c = std::tolower(c);});

                    if (choice.size() > 1 || !(choice[0] == 'y' || choice[0] == 'n' || choice.empty())) {
                        std::cout << "Invalid answer : \"" << choice << "\"." << std::endl;
                    } else if (choice[0] == 'n') {
                        *question->Answer = false;
                    } else {
                        *question->Answer = true;
                    }
                }
            }
            break;
            case GenericQuestionEvent::QuestionType::RemoveUnresolvablePackages: {
                const RemoveUnresolvablePackagesQuestion *question = static_cast<const RemoveUnresolvablePackagesQuestion*>(event);

                std::string choice{'\0'};
                while (choice[0] != 'n' && choice[0] != 'y' && !choice.empty()) {
                    std::cout << "Unable to resolve target(s): { ";
                    for (std::size_t i = 0; i < question->Packages.size(); i++) {
                        std::cout << question->Packages[i].GetName();
                        if (i + 1 == question->Packages.size()) {
                            std::cout << " }" << std::endl;
                        } else {
                            std::cout << ", ";
                        }
                    }

                    std::cout << "Remove from transaction? [Y/n] > " << std::flush;

                    std::getline(std::cin, choice);
                    std::for_each(choice.begin(), choice.end(), [](char &c) -> void {c = std::tolower(c);});

                    if (choice.size() > 1 || !(choice[0] == 'y' || choice[0] == 'n' || choice.empty())) {
                        std::cout << "Invalid answer : \"" << choice << "\"." << std::endl;
                    } else if (choice[0] == 'n') {
                        *question->Answer = false;
                    } else {
                        *question->Answer = true;
                    }
                }
            }
            break;
            case GenericQuestionEvent::QuestionType::SelectProvider: {
                const SelectProviderQuestion *question = static_cast<const SelectProviderQuestion*>(event);

                std::string choice{'\0'};
                while (!choice.empty() && !std::isdigit(choice[0])) {
                    std::cout << "There are " << question->Providers.size() << " providers available for "
                              << question->Provision.GetName() << ":" << std::endl;
                    // TODO: Revamp selection

                    for (int i = 0; i < question->Providers.size(); i++) {
                        std::cout << i << ") " << question->Providers[i].GetDatabase()->GetName() << "/" << question->Providers[i].GetName() << " " << std::flush;
                    }

                    std::cout << "\nSelect which package will provide " << question->Provision.GetName() << " [0-" << question->Providers.size() << " > " << std::flush;

                    std::getline(std::cin, choice);
                    int numChoice{};
                    try {
                        numChoice = std::stoi(choice);
                    } catch (const std::exception &ex) {
                        std::cout << "Invalid answer : \"" << choice << "\"." << std::endl;
                        continue;
                    }

                    if (numChoice < 0 || numChoice > question->Providers.size()) {
                        std::cout << "Invalid answer : \"" << choice << "\"." << std::endl;
                    } else {
                        *question->Answer = numChoice;
                    }
                }
            }
            break;
            case GenericQuestionEvent::QuestionType::ImportKey: {
                const ImportKeyQuestion *question = static_cast<const ImportKeyQuestion*>(event);

                std::string choice{'\0'};
                while (choice[0] != 'n' && choice[0] != 'y' && !choice.empty()) {
                    std::cout << "Import key? ID: " << question->KeyUID << " Fingerprint: " << question->KeyFingerprint
                              << " [Y/n] > " << std::flush;

                    std::getline(std::cin, choice);
                    std::for_each(choice.begin(), choice.end(), [](char &c) -> void {c = std::tolower(c);});

                    if (choice.size() > 1 || !(choice[0] == 'y' || choice[0] == 'n' || choice.empty())) {
                        std::cout << "Invalid answer :\"" << choice << "\"." << std::endl;
                    } else if (choice[0] == 'n') {
                        *question->Answer = false;
                    } else {
                        *question->Answer = true;
                    }
                }
            }
            break;
        };
    });
    
    alpm_option_set_dlcb(ALPM::GetHandle(), [](void *ctx, const char *filename, alpm_download_event_type_t event, void *data) -> void {
            Event::Event::Emit<DownloadEvent>({.Context = ctx, .Filename = filename, .Type = static_cast<DownloadEvent::DownloadType>(std::to_underlying(event)), .Data = data});
    }, nullptr);

    alpm_option_set_questioncb(ALPM::GetHandle(), [](void *ctx, alpm_question_t *question) -> void {
        switch (question->type) {
            case ALPM_QUESTION_INSTALL_IGNOREPKG: {
                alpm_question_install_ignorepkg_t *qst = reinterpret_cast<alpm_question_install_ignorepkg_t*>(question);
                InstallIgnorePackageQuestion installIgnorePackageQuestion{.IgnoredPackage = Package(qst->pkg)};
                installIgnorePackageQuestion.Answer = &qst->install;
                installIgnorePackageQuestion.Question = GenericQuestionEvent::QuestionType::InstallIgnorePackage;
                Event::Event::Emit<GenericQuestionEvent*>(static_cast<GenericQuestionEvent*>(&installIgnorePackageQuestion));
            }
            break;
            case ALPM_QUESTION_REPLACE_PKG: {
                alpm_question_replace_t *qst = reinterpret_cast<alpm_question_replace_t*>(question);
                ReplacePackageQuestion replacePackageQuestion{.DatabaseOfNewPackage = Database(qst->newdb), .NewPackage = Package(qst->newpkg), .OldPackage = Package(qst->oldpkg)};
                replacePackageQuestion.Answer = &qst->replace;
                replacePackageQuestion.Question = GenericQuestionEvent::QuestionType::ReplacePackage;
                Event::Event::Emit<GenericQuestionEvent*>(static_cast<GenericQuestionEvent*>(&replacePackageQuestion));
            }
            break;
            case ALPM_QUESTION_CONFLICT_PKG: {
                alpm_question_conflict_t *qst = reinterpret_cast<alpm_question_conflict_t*>(question);
                ConflictingPackageQuestion conflictingPackageQuestion{.ConflictInfo = Conflict(qst->conflict)};
                conflictingPackageQuestion.Answer = &qst->remove;
                conflictingPackageQuestion.Question = GenericQuestionEvent::QuestionType::ConflictingPackage;
                Event::Event::Emit<GenericQuestionEvent*>(static_cast<GenericQuestionEvent*>(&conflictingPackageQuestion));
            }
            break;
            case ALPM_QUESTION_CORRUPTED_PKG: {
                alpm_question_corrupted_t *qst = reinterpret_cast<alpm_question_corrupted_t*>(question);
                CurruptedPackageQuestion curruptedPackageQuestion{.CurruptedFile = qst->filepath};
                curruptedPackageQuestion.Answer = &qst->remove;
                curruptedPackageQuestion.Question = GenericQuestionEvent::QuestionType::CurruptedPackage;
                Event::Event::Emit<GenericQuestionEvent*>(static_cast<GenericQuestionEvent*>(&curruptedPackageQuestion));
            }
            break;
            case ALPM_QUESTION_REMOVE_PKGS: {
                alpm_question_remove_pkgs_t *qst = reinterpret_cast<alpm_question_remove_pkgs_t*>(question);
                RemoveUnresolvablePackagesQuestion removeUnresolvablePackagesQuestion{.Packages = Utils::ALPMListToVector<Package>(qst->packages)};
                removeUnresolvablePackagesQuestion.Answer = &qst->skip;
                removeUnresolvablePackagesQuestion.Question = GenericQuestionEvent::QuestionType::RemoveUnresolvablePackages;
                Event::Event::Emit<GenericQuestionEvent*>(static_cast<GenericQuestionEvent*>(&removeUnresolvablePackagesQuestion));
            }
            break;
            case ALPM_QUESTION_SELECT_PROVIDER: {
                alpm_question_select_provider_t *qst = reinterpret_cast<alpm_question_select_provider_t*>(question);
                SelectProviderQuestion selectProviderQuestion{.Provision = Dependency(qst->depend), .Providers = Utils::ALPMListToVector<Package>(qst->providers)};
                selectProviderQuestion.Answer = &qst->use_index;
                selectProviderQuestion.Question = GenericQuestionEvent::QuestionType::SelectProvider;
                Event::Event::Emit<GenericQuestionEvent*>(static_cast<GenericQuestionEvent*>(&selectProviderQuestion));
            }
            break;
            case ALPM_QUESTION_IMPORT_KEY: {
                alpm_question_import_key_t *qst = reinterpret_cast<alpm_question_import_key_t*>(question);
                ImportKeyQuestion importKeyQuestion{.KeyFingerprint = qst->fingerprint, .KeyUID = qst->uid};
                importKeyQuestion.Answer = &qst->import;
                importKeyQuestion.Question = GenericQuestionEvent::QuestionType::ImportKey;
                Event::Event::Emit<GenericQuestionEvent*>(static_cast<GenericQuestionEvent*>(&importKeyQuestion));
            }
            break;
        }
    }, nullptr);
}
