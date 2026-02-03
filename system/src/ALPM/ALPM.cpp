#include "ALPM.hpp"

#include "alpm.h"

#include <stdexcept>
#include <format>

#include "Callbacks.hpp"

alpm_handle_t *ALPM::ALPM::s_alpmHandle = nullptr;
std::shared_ptr<ALPM::Transaction> ALPM::ALPM::s_currentTransaction = {};
ALPM::Config ALPM::ALPM::s_config = Config();

auto ALPM::ALPM::Initialize(const std::filesystem::path &root) -> bool {
    if (s_alpmHandle) {
        return false;
    }

    alpm_errno_t err;

    std::filesystem::path dbPath = root / "var/lib/pacman";
    std::filesystem::path configPath = root / "etc/pacman.conf";

    s_alpmHandle = alpm_initialize(root.c_str(), dbPath.c_str(), &err);

    if (!s_alpmHandle) {
        throw std::runtime_error(std::format("Failed to allocate libalpm handle: {}", alpm_strerror(err)));
    }

    // Load global configuration file
    s_config.LoadFile(configPath);

    // Initializing database results in libalpm_register_syncdb being called
    // for all of the databases defined in pacman.conf
    Database::Initialize();

    Callback::Register<QuestionCallback>(1);

    return true;
}

auto ALPM::ALPM::GetCurrentTransaction() -> std::shared_ptr<Transaction> {
    Initialize();
    if (!s_currentTransaction) {
        s_currentTransaction = ::ALPM::Transaction::Create();
    }

    return s_currentTransaction;
}

auto ALPM::ALPM::SetCurrentTransaction(std::shared_ptr<Transaction> transaction) -> void {
    Initialize();
    s_currentTransaction = transaction;
}

auto ALPM::ALPM::ApplyCurrentTransaction() -> void {
    Initialize();
    alpm_list_t *list = new alpm_list_t;

    /* Databse Updates */
    if (!s_currentTransaction->GetDatabaseUpdates().empty()) {
        alpm_list_t *dbList = list;
        
        // Set up first value
        dbList->prev = dbList;
        dbList->next = nullptr;
        dbList->data = s_currentTransaction->GetDatabaseUpdates().at(0).GetHandle();

        for (int i = 1; i < s_currentTransaction->GetDatabaseUpdates().size(); i++) {
            // Sets up new list element for this current iteration
            dbList->next->prev = dbList;
            dbList->next = new alpm_list_t;
            Database database = s_currentTransaction->GetDatabaseUpdates().at(i);

            dbList->next->data = database.GetHandle();
            
            dbList = dbList->next;
        }

        alpm_db_update(s_alpmHandle, list, false);
        alpm_list_free(list);
    }


}

auto ALPM::ALPM::GetError() -> std::string {
    Initialize();
    return alpm_strerror(alpm_errno(s_alpmHandle));
}

auto ALPM::ALPM::GetLocalDatabase() -> Database {
    Initialize();
    return Database(alpm_get_localdb(s_alpmHandle));
}

auto ALPM::ALPM::GetSyncDatabases() -> std::vector<Database> {
    Initialize();
    alpm_list_t *list = alpm_get_syncdbs(s_alpmHandle);
    std::vector<Database> databases;
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        databases.emplace_back(static_cast<alpm_db_t*>(alpm_list_nth(list, i)->data));
    }

    return databases;
}

auto ALPM::ALPM::GetConfig() -> Config {
    Initialize();
    return s_config;
}

auto ALPM::ALPM::GetHandle() -> alpm_handle_t* {
    Initialize();
    return s_alpmHandle;
}
