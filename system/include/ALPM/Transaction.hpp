#pragma once

#include "Package.hpp"
#include "Database.hpp"

#include <vector>

namespace ALPM {
    class Transaction {
            struct Private {inline explicit Private() {}};
        public:
            enum class PackageOperation {
                Install,
                Uninstall,
                Update
            };

            enum class DatabaseOperation {
                Update
            };

            static auto Create() -> std::shared_ptr<Transaction>;

            Transaction(Private);
            ~Transaction() = default;

            auto AddPackageOperation(const Package &package, PackageOperation operation) -> void;
            auto AddDatabaseOperation(const Database &database, DatabaseOperation operation) -> void;

            auto GetPackageUpdates() const -> std::vector<Package>;
            auto GetPackageInstalls() const -> std::vector<Package>;
            auto GetPackageUninstalls() const -> std::vector<Package>;

            auto GetDatabaseUpdates() const -> std::vector<Database>;

        private:

            static auto CheckPackageOperation(std::pair<Package, PackageOperation> package, PackageOperation operation) -> bool;

            std::vector<std::pair<Package, PackageOperation>> m_packageOperations;
            std::vector<std::pair<Database, DatabaseOperation>> m_databaseOperations;
    };
};
