#pragma once

#include <string>
#include <filesystem>
#include <memory>

#include "Transaction.hpp"
#include "Config.hpp"

struct _alpm_handle_t;
typedef struct _alpm_handle_t alpm_handle_t;
struct _alpm_db_t;
typedef struct _alpm_db_t alpm_db_t;

namespace ALPM {
    class ALPM {
        public:
            static auto Initialize(const std::filesystem::path &root = "/") -> bool;

            static auto GetCurrentTransaction() -> std::shared_ptr<Transaction>;

            static auto SetCurrentTransaction(std::shared_ptr<Transaction> transaction) -> void;

            static auto ApplyCurrentTransaction() -> void;

            static auto GetError() -> std::string;

            static auto GetLocalDatabase() -> Database;

            static auto GetSyncDatabases() -> std::vector<Database>;

            static auto GetConfig() -> Config;


            static auto GetHandle() -> alpm_handle_t*;

        private:
            static alpm_handle_t *s_alpmHandle;

            static std::shared_ptr<Transaction> s_currentTransaction;
            static Config s_config;

    };
}  // namespace ALPM
