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
                NoDependencyChecks,
                NoSaveBackups,
                NoDepVersionChecking,
                RemoveDependingPackages,
                Recursive,
                DatabaseOnly,
                NoHooks,
                AllDependencies,
                DownloadOnly,
                NoScriptlets,
                NoConflicts,
                AsNeeded,
                AllExplicit,
                OnlyRemoveUnneeded,
                RecursiveAll,
                NoLock,

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
