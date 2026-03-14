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

            auto GetPackageCache() const -> std::vector<Package>;

            auto GetServers() const -> std::vector<std::string>;
            auto SetServers(const std::vector<std::string> &urls) -> bool;
            auto AddServer(const std::string &url) -> bool;
            auto RemoveServer(const std::string &url) -> int;

            auto GetHandle() const -> alpm_db_t*;

            /* Transactional functions */
            auto MarkUpdate() const -> void;

        private:
            static auto RegisterSyncDatabase(const std::string &treename, const std::bitset<32> &siglevelFlags, const std::vector<std::string> &servers = {}) -> bool;

            alpm_db_t *m_alpmdb;

    };
}  // namespace ALPM
