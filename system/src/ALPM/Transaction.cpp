#include "Transaction.hpp"

#include <ranges>

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

auto Transaction::GetPackageUpdates() const -> std::vector<Package> {
    auto updates = m_packageOperations | std::views::filter([](std::pair<Package, PackageOperation> pair) -> bool {return pair.second == PackageOperation::Update;}) | std::views::keys;

    return std::ranges::to<std::vector<Package>>(updates);
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
