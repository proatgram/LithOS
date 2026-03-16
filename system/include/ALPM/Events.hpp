#pragma once

#include <string>

#include "Package.hpp"
#include "Transaction.hpp"
#include "Dependency.hpp"

struct test;

namespace ALPM {

    /* ALPM Callbacks */
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


    /* ALPM Events */
    struct PackageOperationEvent {
        Package NewPackage;
        Package OldPackage;
        Transaction::PackageOperation OperationType;
    };

    struct OptionalDependencyRemovedEvent {
        Dependency OptionalDependency;
        Package OwningPackage;
    };

    struct ScriptletInfoEvent {
        std::string ScriptletOutput;
    };

    struct PackageDownloadedEvent {
        std::string PackageName;
    };

    struct PacnewCreatedEvent {
        std::string FileName; // Without .pacnew suffix
        bool FromNoUpgrade;
        Package NewPackage;
        Package OldPackage;
    };

    struct PacsaveCreateEvent {
        std::string FileName; // Without .pacsave suffix
        Package OldPackage;
    };

    struct HookRunEvent {
        // TODO: alpm_hook_run_t
    };

    struct PackageRetrieveEvent {
        std::size_t TotalPackages;
        off_t TotalSize;
    };

    struct GenericQuestionEvent {
        enum class QuestionType {
            InstallIgnorePackage,
            ReplacePackage,
            ConflictingPackage,
            CurruptedPackage,
            RemoveUnresolvablePackages,
            SelectProvider,
            ImportKey,
        };

        QuestionType Question;
        int *Answer;
    };

    struct InstallIgnorePackageQuestion : public GenericQuestionEvent {
        Package IgnoredPackage;
    };

    struct ReplacePackageQuestion : public GenericQuestionEvent {
        Database DatabaseOfNewPackage;
        Package NewPackage;
        Package OldPackage;
    };

    struct ConflictingPackageQuestion : public GenericQuestionEvent {
        Conflict ConflictInfo;
    };

    struct CurruptedPackageQuestion : public GenericQuestionEvent {
        std::filesystem::path CurruptedFile; // To remove
        // TODO: Error (alpm_errno_t)
    };

    struct RemoveUnresolvablePackagesQuestion : public GenericQuestionEvent {
        std::vector<Package> Packages;
    };

    struct SelectProviderQuestion : public GenericQuestionEvent {
        Dependency Provision; // What all the packages provide
        std::vector<Package> Providers;
    };

    struct ImportKeyQuestion : public GenericQuestionEvent {
        std::string KeyFingerprint;
        std::string KeyUID;
    };

    class Events {
        public:
            static auto RegisterEvents() -> void;
    };
}  // namespace ALPM
