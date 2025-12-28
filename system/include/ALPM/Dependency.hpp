#pragma once

#include <string>
#include <tuple>
#include <filesystem>
#include <optional>

struct _alpm_depend_t;
typedef _alpm_depend_t alpm_depend_t;
struct _alpm_depmissing_t;
typedef _alpm_depmissing_t alpm_depmissing_t;
struct _alpm_conflict_t;
typedef _alpm_conflict_t alpm_conflict_t;
struct _alpm_fileconflict_t;
typedef _alpm_fileconflict_t alpm_fileconflict_t;

namespace ALPM {
    class Package;
    class Dependency {
        public:
            enum class MatchMode {
                Any,
                Equals,
                GreaterEquals,
                LessEquals,
                Greater,
                Less
            };

            Dependency(alpm_depend_t *depend);
            ~Dependency() = default;

            auto GetDescription() const -> std::string;
            auto GetMatchMode() const -> MatchMode;
            auto GetName() const -> std::string;
            auto GetNameHash() const -> unsigned long;
            auto GetVersion() const -> std::string;

        private:
            alpm_depend_t *m_alpmDepend;
    };

    class MissingDependency {
        public:
            MissingDependency(alpm_depmissing_t *missingDependency);
            ~MissingDependency() = default;

            auto GetCausingPackage() const -> std::string;
            auto GetWantedDependency() const -> Dependency;
            auto GetTargetName() const -> std::string;

        private:
            alpm_depmissing_t *m_alpmDepMissing;

    };

    class Conflict {
        public:
            Conflict(alpm_conflict_t *conflict);
            ~Conflict() = default;

            auto GetPackages() const -> std::tuple<ALPM::Package, ALPM::Package>;
            auto GetReason() const -> Dependency;

        private:

            alpm_conflict_t *m_alpmConflict;

    };

    class FileConflict {
        public:
            enum class ConflictType {
                Target,
                Filesystem
            };

            FileConflict(alpm_fileconflict_t *fileConflict);
            ~FileConflict() = default;

            auto GetConflictingFile() const -> std::filesystem::path;
            auto GetCausingPackageName() const -> std::string;
            auto GetPackageNameAlsoOwns() const -> std::optional<std::string>;
            auto GetConflictType() const -> ConflictType;

        private:
            alpm_fileconflict_t *m_alpmFileConflict;
    };
}
