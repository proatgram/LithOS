#pragma once

#include <filesystem>
#include <vector>
#include <optional>
#include <chrono>

#include "Dependency.hpp"
#include "Database.hpp"
#include "File.hpp"

struct _alpm_pkg_t;
typedef struct _alpm_pkg_t alpm_pkg_t;

namespace ALPM {
    class Package {
        public:
            using UnderlyingType = alpm_pkg_t;

            enum class From {
                File,
                LocalDB,
                SyncDB
            };
            enum class Reason {
                Explicit,
                Depend,
                Unknown
            };
            enum class Validation {
                Unknown,
                None,
                MD5SUM,
                SHA256SUM,
                Signature
            };

            explicit Package(alpm_pkg_t *pkg, bool canFree = false);
            ~Package();

            friend auto operator<(const Package &lhs, const Package &rhs) -> bool;
            friend auto operator>(const Package &lhs, const Package &rhs) -> bool;

            friend auto operator<=(const Package &lhs, const Package &rhs) -> bool;
            friend auto operator>=(const Package &lhs, const Package &rhs) -> bool;

            friend auto operator==(const Package &lhs, const Package &rhs) -> bool;
            friend auto operator!=(const Package &lhs, const Package &rhs) -> bool;

            static auto FromFile(const std::filesystem::path &file, int verificationLevel) -> Package;
            static auto Download(const std::vector<Package> &packages) -> std::vector<std::filesystem::path>;
            auto Download() const -> std::filesystem::path;

            auto CheckMD5Sum() const -> std::optional<std::string>;

            auto GetMD5Sum() const -> std::string;

            auto GetSHA256Sum() const -> std::string;

            auto GetSignature() const -> std::vector<unsigned char>;

            auto ComputeOptionalFor() const -> std::vector<std::string>;

            auto ComputeRequiredBy() const -> std::vector<std::string>;

            auto GetDownloadSize() const -> off_t;

            auto GetSize() const -> off_t;

            auto GetArch() const -> std::string;

            auto GetBaseName() const -> std::string;

            auto GetBase64Signature() const -> std::string;

            auto GetBuildDate() const -> std::chrono::system_clock::time_point;

            auto GetCheckDepends() const -> std::vector<Dependency>;

            auto GetMakeDepends() const -> std::vector<Dependency>;

            auto GetOptionalDepends() const -> std::vector<Dependency>;

            auto GetConflicts() const -> std::vector<Dependency>;

            auto GetDatabase() const -> std::optional<Database>;

            auto GetDepends() const -> std::vector<Dependency>;

            auto GetDescription() const -> std::string;

            auto GetFilename() const -> std::string;

            auto GetFiles() const -> std::vector<File>;

            auto GetGroups() const -> std::vector<std::string>;

            auto GetInstallDate() const -> std::chrono::system_clock::time_point;

            auto GetInstallSize() const -> off_t;

            auto GetLicenses() const -> std::vector<std::string>;

            auto GetName() const -> std::string;

            auto GetOrigin() const -> From;

            auto GetPackager() const -> std::string;

            auto GetProvides() const -> std::vector<Dependency>;

            auto GetReason() const -> Reason;

            auto GetReplaces() const -> std::vector<Dependency>;

            auto GetURL() const -> std::string;

            auto GetValidation() const -> Validation;

            auto GetVersion() const -> std::string;

            auto HasScriptlet() const -> bool;

            auto SetReason(Reason reason) -> void;

            auto ShouldIgnore() const -> bool;


            auto GetHandle() const -> alpm_pkg_t*;

            /* Transactional functions */
            auto MarkInstall() const -> void;
            auto MarkUninstall() const -> void;

        private:
            bool m_canFree;
            auto Free() -> void;

            alpm_pkg_t *m_alpmPkg;
    };
}
