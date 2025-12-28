#include "Package.hpp"
#include "ALPM.hpp"
#include "alpm.h"
#include <format>
#include <stdexcept>
#include <cstring>

using namespace ALPM;

Package::Package(alpm_pkg_t *pkg, bool canFree) :
    m_alpmPkg(pkg), m_canFree(canFree) {}

Package::~Package() {
    Free();
}

auto operator<(const Package &lhs, const Package &rhs) -> bool {
    return alpm_pkg_vercmp(lhs.GetVersion().c_str(), rhs.GetVersion().c_str()) < 0;
}

auto operator>(const Package &lhs, const Package &rhs) -> bool {
    return alpm_pkg_vercmp(lhs.GetVersion().c_str(), rhs.GetVersion().c_str()) > 0;
}

auto operator<=(const Package &lhs, const Package &rhs) -> bool {
    return alpm_pkg_vercmp(lhs.GetVersion().c_str(), rhs.GetVersion().c_str()) <= 0;
}

auto operator>=(const Package &lhs, const Package &rhs) -> bool {
    return alpm_pkg_vercmp(lhs.GetVersion().c_str(), rhs.GetVersion().c_str()) >= 0;
}

auto operator==(const Package &lhs, const Package &rhs) -> bool {
    return alpm_pkg_vercmp(lhs.GetVersion().c_str(), rhs.GetVersion().c_str()) == 0;
}

auto operator!=(const Package &lhs, const Package &rhs) -> bool {
    return alpm_pkg_vercmp(lhs.GetVersion().c_str(), rhs.GetVersion().c_str()) != 0;
}

auto Package::Free() -> void {
    // Packages from a db will be freed upon db unregister,
    // packages from local file must be freed however.
    if (m_canFree) {
        alpm_pkg_free(m_alpmPkg);
    }
}

auto Package::FromFile(const std::filesystem::path &file, int verificationLevel) -> Package {
    alpm_pkg_t *pkg = nullptr;
    if (alpm_pkg_load(ALPM::GetHandle(), file.c_str(), true, verificationLevel, &pkg) != 0) {
        throw std::runtime_error(std::format("Failed to load package from file {}: {}", file.string(), ALPM::GetError()));
    }

    return Package(pkg, true);
}

auto Package::Download(const std::vector<Package> &packages) -> std::vector<std::filesystem::path> {
    alpm_list_t *list;

    for (const Package &package : packages) {
        char *url = new char[package.GetURL().size()];
        std::strcpy(url, package.GetURL().c_str());
        alpm_list_add(list, url);
    }

    alpm_list_t *fetched = nullptr;
    alpm_fetch_pkgurl(ALPM::GetHandle(), list, &fetched);

    std::vector<std::filesystem::path> paths;
    for (size_t i = 0; i < alpm_list_count(fetched); i++) {
        paths.push_back(static_cast<char*>(alpm_list_nth(fetched, 0)->data));
    }

    FREELIST(list);
    FREELIST(fetched);

    return paths;
}

auto Package::Download() const -> std::filesystem::path {
    alpm_list_t *list = new alpm_list_t;
    list->prev = list;
    list->next = nullptr;
    std::string url = GetURL();
    list->data = new char[url.size()];
    std::strcpy(static_cast<char*>(list->data), url.data());

    alpm_list_t *fetched = nullptr;
    alpm_fetch_pkgurl(ALPM::GetHandle(), list, &fetched);

    std::filesystem::path path = static_cast<char*>(fetched->data);

    FREELIST(list);
    FREELIST(fetched);

    return path;
}

auto Package::CheckMD5Sum() const -> std::optional<std::string> {
    if (alpm_pkg_checkmd5sum(m_alpmPkg) != 0) {
        return ALPM::GetError();
    }

    return {};
}

auto Package::GetMD5Sum() const -> std::string {
    return alpm_pkg_get_md5sum(m_alpmPkg);
}

auto Package::GetSHA256Sum() const -> std::string {
    return alpm_pkg_get_sha256sum(m_alpmPkg);
}

auto Package::GetSignature() const -> std::vector<unsigned char> {
    unsigned char *sig = nullptr;
    size_t length;
    if (alpm_pkg_get_sig(m_alpmPkg, &sig, &length) != 0) {

    }

    std::vector<unsigned char> signature(sig, sig + length);
    free(sig);

    return signature;
}

auto Package::ComputeOptionalFor() const -> std::vector<std::string> {
    alpm_list_t *optionalFor = alpm_pkg_compute_optionalfor(m_alpmPkg);

    std::vector<std::string> packages;
    for (size_t i = 0; i < alpm_list_count(optionalFor); i++) {
        packages.emplace_back(static_cast<char*>(alpm_list_nth(optionalFor, i)->data));
    }

    FREELIST(optionalFor);

    return packages;
}

auto Package::ComputeRequiredBy() const -> std::vector<std::string> {
    alpm_list_t *requiredBy = alpm_pkg_compute_requiredby(m_alpmPkg);

    std::vector<std::string> packages;
    for (size_t i = 0; alpm_list_count(requiredBy); i++) {
        packages.emplace_back(static_cast<char*>(alpm_list_nth(requiredBy, i)->data));
    }

    FREELIST(requiredBy);

    return packages;
}

auto Package::GetDownloadSize() const -> off_t {
    return alpm_pkg_download_size(m_alpmPkg);
}

auto Package::GetSize() const -> off_t {
    return alpm_pkg_get_size(m_alpmPkg);
}

auto Package::GetArch() const -> std::string {
    return alpm_pkg_get_arch(m_alpmPkg);
}

auto Package::GetBaseName() const -> std::string {
    return alpm_pkg_get_base(m_alpmPkg);
}

auto Package::GetBase64Signature() const -> std::string {
    return alpm_pkg_get_base64_sig(m_alpmPkg);
}

auto Package::GetBuildDate() const -> std::chrono::system_clock::time_point {
    std::chrono::seconds epochTime(alpm_pkg_get_builddate(m_alpmPkg));
    return std::chrono::system_clock::time_point(epochTime);
}

auto Package::GetCheckDepends() const -> std::vector<Dependency> {
    std::vector<Dependency> depends;

    alpm_list_t *checkDepends = alpm_pkg_get_checkdepends(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(checkDepends); i++) {
        depends.emplace_back(static_cast<alpm_depend_t*>(alpm_list_nth(checkDepends, i)->data));
    }

    return depends;
}

auto Package::GetMakeDepends() const -> std::vector<Dependency> {
    std::vector<Dependency> depends;
    alpm_list_t *list = alpm_pkg_get_makedepends(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        depends.emplace_back(static_cast<alpm_depend_t*>(alpm_list_nth(list, i)->data));
    }
    return depends;
}

auto Package::GetOptionalDepends() const -> std::vector<Dependency> {
    std::vector<Dependency> depends;
    alpm_list_t *list = alpm_pkg_get_optdepends(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        depends.emplace_back(static_cast<alpm_depend_t*>(alpm_list_nth(list, i)->data));
    }

    return depends;
}

auto Package::GetConflicts() const -> std::vector<Dependency> {
    std::vector<Dependency> conflicts;

    alpm_list_t *confs = alpm_pkg_get_conflicts(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(confs); i++) {
        conflicts.emplace_back(static_cast<alpm_depend_t*>(alpm_list_nth(confs, i)->data));
    }

    return conflicts;
}

auto Package::GetDatabase() const -> std::optional<Database> {
    alpm_db_t *db = alpm_pkg_get_db(m_alpmPkg);
    if (db) {
        return Database(db);
    }

    return {};
}

auto Package::GetDepends() const -> std::vector<Dependency> {
    std::vector<Dependency> depends;

    alpm_list_t *list = alpm_pkg_get_depends(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        depends.emplace_back(static_cast<alpm_depend_t*>(alpm_list_nth(list, i)->data));
    }

    return depends;
}

auto Package::GetDescription() const -> std::string {
    return alpm_pkg_get_desc(m_alpmPkg);
}

auto Package::GetFilename() const -> std::string {
    return alpm_pkg_get_filename(m_alpmPkg);
}

auto Package::GetFiles() const -> std::vector<File> {
    std::vector<File> files;
    alpm_filelist_t *fileList = alpm_pkg_get_files(m_alpmPkg);
    for (size_t i = 0; i < fileList->count; i++) {
        File file(&fileList->files[i]);
    }

    return files;
}

auto Package::GetGroups() const -> std::vector<std::string> {
    std::vector<std::string> groups;
    alpm_list_t *list = alpm_pkg_get_groups(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        groups.push_back(static_cast<char*>(alpm_list_nth(list, i)->data));
    }

    return groups;
}

auto Package::GetInstallDate() const -> std::chrono::system_clock::time_point {
    std::chrono::seconds epochTime(alpm_pkg_get_installdate(m_alpmPkg));

    return std::chrono::system_clock::time_point(epochTime);
}

auto Package::GetInstallSize() const -> off_t {
    return alpm_pkg_get_isize(m_alpmPkg);
}

auto Package::GetLicenses() const -> std::vector<std::string> {
    std::vector<std::string> licenses;
    alpm_list_t *list = alpm_pkg_get_licenses(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        licenses.emplace_back(static_cast<char*>(alpm_list_nth(list, i)->data));
    }

    return licenses;
}

auto Package::GetName() const -> std::string {
    return alpm_pkg_get_name(m_alpmPkg);
}

auto Package::GetOrigin() const -> From {
    switch (alpm_pkg_get_origin(m_alpmPkg)) {
        case ALPM_PKG_FROM_FILE:
            return From::File;
        case ALPM_PKG_FROM_SYNCDB:
            return From::SyncDB;
        case ALPM_PKG_FROM_LOCALDB:
            return From::LocalDB;
        default:
            throw std::runtime_error("Unknown libalpm origin.");
    }
}

auto Package::GetPackager() const -> std::string {
    return alpm_pkg_get_packager(m_alpmPkg);
}

auto Package::GetProvides() const -> std::vector<Dependency> {
    std::vector<Dependency> provides;
    alpm_list_t *list = alpm_pkg_get_provides(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        provides.emplace_back(static_cast<alpm_depend_t*>(alpm_list_nth(list, i)->data));
    }
    return provides;
}

auto Package::GetReason() const -> Reason {
    switch (alpm_pkg_get_reason(m_alpmPkg)) {
        case ALPM_PKG_REASON_DEPEND:
            return Reason::Depend;
        case ALPM_PKG_REASON_EXPLICIT:
            return Reason::Explicit;
        case ALPM_PKG_REASON_UNKNOWN:
            return Reason::Unknown;
        default:
            throw std::runtime_error("Unknown libalpm install reason.");
    }
}

auto Package::GetReplaces() const -> std::vector<Dependency> {
    std::vector<Dependency> replaces;
    alpm_list_t *list = alpm_pkg_get_replaces(m_alpmPkg);
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        replaces.emplace_back(static_cast<alpm_depend_t*>(alpm_list_nth(list, i)->data));
    }

    return replaces;
}

auto Package::GetURL() const -> std::string {
    return alpm_pkg_get_url(m_alpmPkg);
}

auto Package::GetValidation() const -> Validation {
    switch (alpm_pkg_get_validation(m_alpmPkg)) {
        case ALPM_PKG_VALIDATION_NONE:
            return Validation::None;
        case ALPM_PKG_VALIDATION_MD5SUM:
            return Validation::MD5SUM;
        case ALPM_PKG_VALIDATION_SHA256SUM:
            return Validation::SHA256SUM;
        case ALPM_PKG_VALIDATION_SIGNATURE:
            return Validation::Signature;
        case ALPM_PKG_VALIDATION_UNKNOWN:
            return Validation::Unknown;
        default:
            throw std::runtime_error("Unknown libalpm package validation method.");
    }
}

auto Package::GetVersion() const -> std::string {
    return alpm_pkg_get_version(m_alpmPkg);
}

auto Package::HasScriptlet() const -> bool {
    return alpm_pkg_has_scriptlet(m_alpmPkg) == 0 ? false : true;
}

auto Package::SetReason(Reason reason) -> void {
    int ret{0};
    switch (reason) {
        case Reason::Explicit:
            ret = alpm_pkg_set_reason(m_alpmPkg, ALPM_PKG_REASON_EXPLICIT);
            break;
        case Reason::Depend:
            ret = alpm_pkg_set_reason(m_alpmPkg, ALPM_PKG_REASON_DEPEND);
            break;
        case Reason::Unknown:
            ret = alpm_pkg_set_reason(m_alpmPkg, ALPM_PKG_REASON_UNKNOWN);
            break;
        default:
            throw std::runtime_error("Unknown Package Reason given.");
    }

    if (ret != 0) {
        throw std::runtime_error(std::format("Failed to set package install reason: {}", ALPM::GetError()));
    }
}

auto Package::ShouldIgnore() const -> bool {
    return alpm_pkg_should_ignore(ALPM::GetHandle(), m_alpmPkg) == 1 ? false : true;
}

auto Package::GetHandle() const -> alpm_pkg_t* {
    return m_alpmPkg;
}

auto Package::MarkInstall() const -> void {
    ALPM::GetCurrentTransaction()->AddPackageOperation(*this, Transaction::PackageOperation::Install);
}

auto Package::MarkUninstall() const -> void {
    ALPM::GetCurrentTransaction()->AddPackageOperation(*this, Transaction::PackageOperation::Uninstall);
}

auto Package::MarkUpdate() const -> void {
    ALPM::GetCurrentTransaction()->AddPackageOperation(*this, Transaction::PackageOperation::Update);
}
