#pragma once

#include <chrono>
#include <string>
#include <expected>

struct _alpm_pgpkey_t;
typedef _alpm_pgpkey_t alpm_pgpkey_t;

namespace ALPM {
    class Package;
    class Database;

    class PGPKey {
        public:
            enum class Status {
                Valid,
                Invalid,
                KeyExpired,
                SignatureExpired,
                KeyUnknown,
                KeyDisabled
            };
            enum class Validity {
                Trusted,
                Marginal,
                Untrusted,
                Unknown
            };

            PGPKey(alpm_pgpkey_t key, Status status, Validity validity);

            PGPKey(const PGPKey &other);
            PGPKey(PGPKey &&other);

            auto operator=(const PGPKey &other) -> PGPKey&;
            auto operator=(PGPKey &&other) -> PGPKey&;

            ~PGPKey();

            auto GetCreatedTime() const -> std::chrono::system_clock::time_point;
            auto GetData() const -> void*;
            auto GetEmail() const -> std::string;
            auto GetExpiresTime() const -> std::chrono::system_clock::time_point;
            auto GetFingerprint() const -> std::string;
            auto GetDataLength() const -> uint32_t;
            auto GetOwnerName() const -> std::string;
            auto IsRevoked() const -> bool;
            auto GetUID() const -> std::string;

            auto GetStatus() const -> Status;
            auto GetValidity() const -> Validity;

        private:
            std::unique_ptr<alpm_pgpkey_t> m_alpmPGPKey;

            Status m_status = Status::KeyUnknown;
            Validity m_validity = Validity::Unknown;
    };

    class Signature {
        public:
            // Can be safely casted to libalpm type, has same values
            enum class Level {
                PackageRequires = 1,
                PackageOptional = 2,
                PackageMarginalOk = 3,
                PackageUnknownOk = 4,
                DatabaseRequires = 11,
                DatabaseOptional = 12,
                DatabaseMarginalOk = 13,
                DatabaseUnknownOk = 14,
                UseDefault = 31
            };

            friend auto operator|(const Level &lhs, const Level &rhs) -> Level;

            static auto CheckSignature(const Package &package) -> std::expected<std::vector<PGPKey>, std::pair<std::string, int>>;
            static auto CheckSignature(const Database &database) -> std::expected<std::vector<PGPKey>, std::pair<std::string, int>>;

        private:
    };
}  // namespace ALPM
