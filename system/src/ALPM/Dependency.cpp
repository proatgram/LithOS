#include "Dependency.hpp"
#include "Package.hpp"

#include "alpm.h"

#include <stdexcept>

using namespace ALPM;

Dependency::Dependency(alpm_depend_t *depend) :
    m_alpmDepend(depend) {}

auto Dependency::GetDescription() const -> std::string {
    return m_alpmDepend->desc;
}

auto Dependency::GetMatchMode() const -> MatchMode {
    switch (m_alpmDepend->mod) {
        case ALPM_DEP_MOD_ANY:
            return MatchMode::Any;
        case ALPM_DEP_MOD_EQ:
            return MatchMode::Equals;
        case ALPM_DEP_MOD_GE:
            return MatchMode::GreaterEquals;
        case ALPM_DEP_MOD_LE:
            return MatchMode::LessEquals;
        case ALPM_DEP_MOD_GT:
            return MatchMode::Greater;
        case ALPM_DEP_MOD_LT:
            return MatchMode::Less;
        default:
            throw std::runtime_error("Unknown libalpm dependency mode.");
    }
}

auto Dependency::GetName() const -> std::string {
    return m_alpmDepend->name;
}

auto Dependency::GetNameHash() const -> unsigned long {
    return m_alpmDepend->name_hash;
}

auto Dependency::GetVersion() const -> std::string {
    return m_alpmDepend->version;
}

auto Dependency::GetHandle() const -> Dependency::UnderlyingType* {
    return m_alpmDepend;
}

MissingDependency::MissingDependency(alpm_depmissing_t *missingDependency) :
    m_alpmDepMissing(missingDependency) {}

auto MissingDependency::GetCausingPackage() const -> std::string {
    return m_alpmDepMissing->causingpkg;
}

auto MissingDependency::GetWantedDependency() const -> Dependency {
    return m_alpmDepMissing->depend;
}

auto MissingDependency::GetTargetName() const -> std::string {
    return m_alpmDepMissing->target;
}

auto MissingDependency::GetHandle() const -> MissingDependency::UnderlyingType* {
    return m_alpmDepMissing;
}

Conflict::Conflict(alpm_conflict_t *conflict) :
    m_alpmConflict(conflict) {}

auto Conflict::GetPackages() const -> std::tuple<ALPM::Package, ALPM::Package> {
    return std::make_pair<ALPM::Package, ALPM::Package>(ALPM::Package(m_alpmConflict->package1), ALPM::Package(m_alpmConflict->package2));
}

auto Conflict::GetReason() const -> Dependency {
    return m_alpmConflict->reason;
}

auto Conflict::GetHandle() const -> Conflict::UnderlyingType* {
    return m_alpmConflict;
}

FileConflict::FileConflict(alpm_fileconflict_t *fileConflict) :
    m_alpmFileConflict(fileConflict) {}

auto FileConflict::GetConflictingFile() const -> std::filesystem::path {
    return m_alpmFileConflict->file;
}

auto FileConflict::GetCausingPackageName() const -> std::string {
    return m_alpmFileConflict->target;
}

auto FileConflict::GetPackageNameAlsoOwns() const -> std::optional<std::string> {
    return m_alpmFileConflict->ctarget;
}

auto FileConflict::GetConflictType() const -> ConflictType {
    switch (m_alpmFileConflict->type) {
        case ALPM_FILECONFLICT_TARGET:
            return ConflictType::Target;
        case ALPM_FILECONFLICT_FILESYSTEM:
            return ConflictType::Filesystem;
        default:
            throw std::runtime_error("Unknown libalpm file conflict type.");
    }
}

auto FileConflict::GetHandle() const -> FileConflict::UnderlyingType* {
    return m_alpmFileConflict;
}
