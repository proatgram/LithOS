#include "Transaction.hpp"
#include "ALPM.hpp"

#include <alpm.h>

#include <ranges>
#include <format>

using namespace ALPM;

Transaction::Transaction(Transaction::Private) {}

auto Transaction::Create() -> std::shared_ptr<Transaction> {
    return std::make_shared<Transaction>(Private());
}

auto Transaction::AddPackageOperation(const Package &package, PackageOperation operation) -> void {
    m_packageOperations.emplace_back(Package(package.GetHandle()), operation);
}

auto Transaction::AddDatabaseOperation(const Database &database, DatabaseOperation operation) -> void {
    m_databaseOperations.emplace_back(Database(database.GetHandle()), operation);
}

auto Transaction::AddSystemUpgradeOperation() -> void {
    m_systemUpgrade = true;
}

auto Transaction::GetPackageInstalls() const -> std::vector<Package> {
    auto updates = m_packageOperations | std::views::filter([](std::pair<Package, PackageOperation> pair) -> bool {return pair.second == PackageOperation::Install;}) | std::views::keys;

    return std::ranges::to<std::vector<Package>>(updates);
}

auto Transaction::GetPackageUninstalls() const -> std::vector<Package> {
    auto updates = m_packageOperations | std::views::filter([](std::pair<Package, PackageOperation> pair) -> bool {return pair.second == PackageOperation::Uninstall;}) | std::views::keys;

    return std::ranges::to<std::vector<Package>>(updates);
}

auto Transaction::GetDatabaseUpdates() const -> std::vector<Database> {
    auto updates = m_databaseOperations | std::views::filter([](std::pair<Database, DatabaseOperation> pair) -> bool {return pair.second == DatabaseOperation::Update;}) | std::views::keys;

    return std::ranges::to<std::vector<Database>>(updates);
}

auto Transaction::Apply() const -> void {
    if (alpm_trans_init(ALPM::GetHandle(), static_cast<alpm_transflag_t>(m_transactionFlags.to_ulong() & 0b111111111111111111)) != 0) {
        throw std::runtime_error(std::format("Failed to apply transaction: Failed to initialize libalpm transaction: {}", ALPM::GetError()));
    }

    for (const auto &[package, operation] : m_packageOperations) {
        switch (operation) {
            case PackageOperation::Install:
                if (alpm_add_pkg(ALPM::GetHandle(), package.GetHandle()) != 0) {
                    throw std::runtime_error(std::format("Failed to apply transaction: Couldn't add package to libalpm transaction: {}", ALPM::GetError()));
                }
                break;
            case PackageOperation::Uninstall:
                if (alpm_remove_pkg(ALPM::GetHandle(), package.GetHandle()) != 0) {
                    throw std::runtime_error(std::format("Failed to apply transaction: Couldn't add package to libalpm transaction: {}", ALPM::GetError()));
                }
                break;
        }
    }

    
    if (!GetDatabaseUpdates().empty()) {
        alpm_list_t *list = new alpm_list_t;
        list->prev = nullptr;
        alpm_list_t *dbList = list;
        for (const Database &database : GetDatabaseUpdates()) {
            // Handle first list entry
            if (dbList == list) {
                dbList->data = database.GetHandle();
                continue;
            }

            // Sets up new entry in list
            dbList->next = new alpm_list_t;
            dbList = dbList->next;

            dbList->data = database.GetHandle();
        }

        // TODO: Allow modifying force flag
        if (alpm_db_update(ALPM::GetHandle(), list, false) != 0) {
            alpm_list_free(list);
            dbList = nullptr;

            throw std::runtime_error(std::format("Failed to apply transaction: Could not add database updates to libalpm transaction: {}", ALPM::GetError()));
        }
    }

    if (m_systemUpgrade) {
        if (alpm_sync_sysupgrade(ALPM::GetHandle(), false) != 0) {
            throw std::runtime_error(std::format("Failed to apply transaction: Could not add packages to upgrade: {}", ALPM::GetError()));
        }
    }

    alpm_list_t *errorList = nullptr;
    if (alpm_trans_prepare(ALPM::GetHandle(), &errorList) != 0) {
        if (errorList) {
            // TODO: Handle conflicting packages

            alpm_list_free(errorList);
            errorList = nullptr;
        }
        throw std::runtime_error(std::format("Failed to prepare transaction: {}", ALPM::GetError()));
    }

    if (errorList) {
        // TODO: Handle conflicting packages

        alpm_list_free(errorList);
        errorList = nullptr;
    }

    
    if (alpm_trans_commit(ALPM::GetHandle(), &errorList) != 0) {
        if (errorList) {
            // TODO: Handle possible errors

            alpm_list_free(errorList);
            errorList = nullptr;
        }

        throw std::runtime_error(std::format("Failed to commit transaction: {}", ALPM::GetError()));
    }
}
