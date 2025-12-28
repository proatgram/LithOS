#pragma once

#include <string>
#include <optional>
#include <vector>
#include <bitset>

#include "Signature.hpp"

struct _alpm_db_t;
typedef struct _alpm_db_t alpm_db_t;

namespace ALPM {
    class Package;
    class Database {
        public:
            explicit Database(alpm_db_t *db);
            ~Database() = default;

            static auto Initialize() -> void;
            auto GetName() const -> std::string;
            auto GetSigLevel() const -> int;
            auto IsValid() const -> std::optional<std::string>;

            auto GetPackage(const std::string &name) const -> Package;
            auto Search(const std::string &expression) const -> std::vector<Package>;

            auto GetHandle() const -> alpm_db_t*;

            /* Transactional functions */
            auto MarkUpdate() const -> void;

        private:
            static auto RegisterSyncDatabase(const std::string &treename, const std::bitset<32> &siglevelFlags) -> bool;

            alpm_db_t *m_alpmdb;

    };
}  // namespace ALPM
