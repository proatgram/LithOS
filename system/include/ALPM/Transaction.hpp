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
                Upgrade,
                Reinstall,
                Downgrade,
                Uninstall
            };

            enum class DatabaseOperation {
                Update
            };

            enum class OperationFlags {
                /* libalpm flags */
                NoDependencyChecks = 0,
                NoSaveBackups = 2,
                NoDepVersionChecking = 3,
                RemoveDependingPackages = 4,
                Recursive = 5,
                DatabaseOnly = 6,
                NoHooks = 7,
                AllDependencies = 8,
                DownloadOnly = 9,
                NoScriptlets = 10,
                NoConflicts = 11,
                AsNeeded = 13,
                AllExplicit = 14,
                OnlyRemoveUnneeded = 15,
                RecursiveAll = 16,
                NoLock = 17,

                /* Wrapper flags */
                ForceDatabase
            };

            static auto Create() -> std::shared_ptr<Transaction>;

            Transaction(Private);
            ~Transaction();

            template <typename ...Args>
            inline auto SetFlags(OperationFlags flag, Args... flags) -> void {
                m_transactionFlags.set(static_cast<size_t>(flag));
                (m_transactionFlags.set(static_cast<size_t>(flags)), ...);
            }
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
