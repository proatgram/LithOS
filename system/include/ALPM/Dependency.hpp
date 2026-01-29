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
            using UnderlyingType = alpm_depend_t;
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

            auto GetHandle() const -> UnderlyingType*;

        private:
            UnderlyingType *m_alpmDepend;
    };

    class MissingDependency {
        public:
            using UnderlyingType = alpm_depmissing_t;
            MissingDependency(alpm_depmissing_t *missingDependency);
            ~MissingDependency() = default;

            auto GetCausingPackage() const -> std::string;
            auto GetWantedDependency() const -> Dependency;
            auto GetTargetName() const -> std::string;

            auto GetHandle() const -> UnderlyingType*;

        private:
            UnderlyingType *m_alpmDepMissing;

    };

    class Conflict {
        public:
            using UnderlyingType = alpm_conflict_t;
            Conflict(alpm_conflict_t *conflict);
            ~Conflict() = default;

            auto GetPackages() const -> std::tuple<ALPM::Package, ALPM::Package>;
            auto GetReason() const -> Dependency;

            auto GetHandle() const -> UnderlyingType*;

        private:

            UnderlyingType *m_alpmConflict;

    };

    class FileConflict {
        public:
            using UnderlyingType = alpm_fileconflict_t;
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

            auto GetHandle() const -> UnderlyingType*;

        private:
            UnderlyingType *m_alpmFileConflict;
    };
}
