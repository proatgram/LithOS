#include "Signature.hpp"
#include "ALPM.hpp"
#include "Package.hpp"
#include "Database.hpp"

#include "alpm.h"

using namespace ALPM;

PGPKey::PGPKey(alpm_pgpkey_t key, Status status, Validity validity) :
    m_alpmPGPKey(std::make_unique<alpm_pgpkey_t>(key)), 
    m_status(status),
    m_validity(validity) {}

PGPKey::~PGPKey() {
    delete m_alpmPGPKey->name;
    delete m_alpmPGPKey->fingerprint;
    delete m_alpmPGPKey->email;
    free(m_alpmPGPKey->data); // Have to use C free() here as type is void*.
    delete m_alpmPGPKey->uid;
}

auto PGPKey::GetCreatedTime() const -> std::chrono::system_clock::time_point {
    return std::chrono::system_clock::time_point(std::chrono::seconds(m_alpmPGPKey->created));
}

auto PGPKey::GetData() const -> void* {
    return m_alpmPGPKey->data;
}

auto PGPKey::GetEmail() const -> std::string {
    return m_alpmPGPKey->email;
}

auto PGPKey::GetExpiresTime() const -> std::chrono::system_clock::time_point {
    return std::chrono::system_clock::time_point(std::chrono::seconds(m_alpmPGPKey->expires));
}

auto PGPKey::GetFingerprint() const -> std::string {
    return m_alpmPGPKey->fingerprint;
}

auto PGPKey::GetDataLength() const -> uint32_t {
    return m_alpmPGPKey->length;
}

auto PGPKey::GetOwnerName() const -> std::string {
    return m_alpmPGPKey->name;
}

auto PGPKey::IsRevoked() const -> bool {
    return m_alpmPGPKey->revoked;
}

auto PGPKey::GetUID() const -> std::string {
    return m_alpmPGPKey->uid;
}

auto PGPKey::GetStatus() const -> PGPKey::Status {
    return m_status;
}

auto PGPKey::GetValidity() const -> PGPKey::Validity {
    return m_validity;
}

auto operator|(const Signature::Level &lhs, const Signature::Level &rhs) -> Signature::Level {
    return static_cast<Signature::Level>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

auto Signature::CheckSignature(const Package &package) -> std::expected<std::vector<PGPKey>, std::pair<std::string, int>> {
    alpm_siglist_t siglist;
    std::vector<PGPKey> keyResults;
    int rc = alpm_pkg_check_pgp_signature(package.GetHandle(), &siglist);

    if (rc != 0) {
        return std::unexpected(std::make_pair(ALPM::GetError(), alpm_errno(ALPM::GetHandle())));
    }

    for (size_t i = 0; i < siglist.count; i++) {
        alpm_sigresult_t &result = siglist.results[i];
        PGPKey::Status status;
        PGPKey::Validity validity;
        switch (result.status) {
            case ALPM_SIGSTATUS_VALID:
                status = PGPKey::Status::Valid;
                break;
            case ALPM_SIGSTATUS_KEY_EXPIRED:
                status = PGPKey::Status::KeyExpired;
                break;
            case ALPM_SIGSTATUS_SIG_EXPIRED:
                status = PGPKey::Status::SignatureExpired;
                break;
            case ALPM_SIGSTATUS_KEY_UNKNOWN:
                status = PGPKey::Status::KeyUnknown;
                break;
            case ALPM_SIGSTATUS_KEY_DISABLED:
                status = PGPKey::Status::KeyDisabled;
                break;
            case ALPM_SIGSTATUS_INVALID:
                status = PGPKey::Status::Invalid;
            default:
                throw std::runtime_error("Invalid PGP Key Status caught.");
        }
        switch (result.validity) {
            case ALPM_SIGVALIDITY_FULL:
                validity = PGPKey::Validity::Trusted;
                break;
            case ALPM_SIGVALIDITY_MARGINAL:
                validity = PGPKey::Validity::Marginal;
                break;
            case ALPM_SIGVALIDITY_NEVER:
                validity = PGPKey::Validity::Untrusted;
                break;
            case ALPM_SIGVALIDITY_UNKNOWN:
                validity = PGPKey::Validity::Unknown;
                break;
        }

        keyResults.emplace_back(result.key, status, validity);
    }

    return keyResults;
}

auto Signature::CheckSignature(const Database &database) -> std::expected<std::vector<PGPKey>, std::pair<std::string, int>> {
    alpm_siglist_t siglist;
    std::vector<PGPKey> keyResults;
    int rc = alpm_db_check_pgp_signature(database.GetHandle(), &siglist);

    if (rc != 0) {
        return std::unexpected(std::make_pair(ALPM::GetError(), alpm_errno(ALPM::GetHandle())));
    }

    for (size_t i = 0; i < siglist.count; i++) {
        alpm_sigresult_t &result = siglist.results[i];
        PGPKey::Status status;
        PGPKey::Validity validity;
        switch (result.status) {
            case ALPM_SIGSTATUS_VALID:
                status = PGPKey::Status::Valid;
                break;
            case ALPM_SIGSTATUS_KEY_EXPIRED:
                status = PGPKey::Status::KeyExpired;
                break;
            case ALPM_SIGSTATUS_SIG_EXPIRED:
                status = PGPKey::Status::SignatureExpired;
                break;
            case ALPM_SIGSTATUS_KEY_UNKNOWN:
                status = PGPKey::Status::KeyUnknown;
                break;
            case ALPM_SIGSTATUS_KEY_DISABLED:
                status = PGPKey::Status::KeyDisabled;
                break;
            case ALPM_SIGSTATUS_INVALID:
                status = PGPKey::Status::Invalid;
            default:
                throw std::runtime_error("Invalid PGP Key Status caught.");
        }
        switch (result.validity) {
            case ALPM_SIGVALIDITY_FULL:
                validity = PGPKey::Validity::Trusted;
                break;
            case ALPM_SIGVALIDITY_MARGINAL:
                validity = PGPKey::Validity::Marginal;
                break;
            case ALPM_SIGVALIDITY_NEVER:
                validity = PGPKey::Validity::Untrusted;
                break;
            case ALPM_SIGVALIDITY_UNKNOWN:
                validity = PGPKey::Validity::Unknown;
                break;
        }

        keyResults.emplace_back(result.key, status, validity);
    }

    return keyResults;
}
