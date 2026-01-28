#pragma once

#include "Package.hpp"
#include "Database.hpp"

#include <vector>
#include <bitset>
#include <queue>

namespace ALPM {
    class Transaction {
            struct Private {inline explicit Private() {}};
        public:
            enum class PackageOperation {
                Install,
                Uninstall
            };

            enum class DatabaseOperation {
                Update
            };

            enum class OperationFlags {
                /* libalpm flags */
                NoDependencyChecks = 1 << 0,
                NoSaveBackups = 1 << 2,
                NoDepVersionChecking = 1 << 3,
                RemoveDependingPackages = 1 << 4,
                Recursive = 1 << 5,
                DatabaseOnly = 1 << 6,
                NoHooks = 1 << 7,
                AllDependencies = 1 << 8,
                DownloadOnly = 1 << 9,
                NoScriptlets = 1 << 10,
                NoConflicts = 1 << 11,
                AsNeeded = 1 << 13,
                AllExplicit = 1 << 14,
                OnlyRemoveUnneeded = 1 << 15,
                RecursiveAll = 1 << 16,
                NoLock = 1 << 17

                /* Wrapper flags */
            };

            static auto Create() -> std::shared_ptr<Transaction>;
            static auto ApplyAll() -> void;

            Transaction(Private);
            ~Transaction() = default;

            auto SetFlags(const std::bitset<32> &transactionFlags);
            auto AddPackageOperation(const Package &package, PackageOperation operation) -> void;
            auto AddDatabaseOperation(const Database &database, DatabaseOperation operation) -> void;
            auto AddSystemUpgradeOperation() -> void;

            auto GetPackageInstalls() const -> std::vector<Package>;
            auto GetPackageUninstalls() const -> std::vector<Package>;

            auto GetDatabaseUpdates() const -> std::vector<Database>;

            auto Apply() const -> void;

        private:
            bool m_valid{true};
            inline static std::queue<std::shared_ptr<Transaction>> s_transactions{};
            static auto CheckPackageOperation(std::pair<Package, PackageOperation> package, PackageOperation operation) -> bool;

            std::bitset<32> m_transactionFlags;

            std::vector<std::pair<Package, PackageOperation>> m_packageOperations;
            std::vector<std::pair<Database, DatabaseOperation>> m_databaseOperations;

            bool m_systemUpgrade{false};
    };
};
