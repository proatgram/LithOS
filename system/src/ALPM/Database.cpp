#include "Database.hpp"
#include "ALPM.hpp"
#include "Package.hpp"
#include "Utils.hpp"

#include "alpm.h"

#include <cstring>
#include <format>
#include <regex>

using namespace ALPM;

Database::Database(alpm_db_t *db) :
    m_alpmdb(db) {}

auto Database::Initialize() -> void {
    Config config = ALPM::GetConfig();
    std::bitset<32> defaultSigLevel = config.GetSection("options").GetOption("SigLevel").As<Signature::Level>();

    for (const Config::Section &section : config.GetSections()) {
        if (section.GetName() == "options") {
            continue;
        }
        std::vector<std::string> servers;
        if (section.HasOption("Server")) {
            for (const Config::Section::Value &server : section.GetOptions("Server")) {
                std::string serverString = server.As<std::string>();
                serverString = std::regex_replace(serverString, std::regex("\\$repo"), section.GetName());
                serverString = std::regex_replace(serverString, std::regex("\\$arch"), Utils::GetSystemArchitecture());
                servers.push_back(serverString);
            }
        }
        
        if (!RegisterSyncDatabase(section.GetName(), (section.HasOption("SigLevel") ? section.GetOption("SigLevel")->As<Signature::Level>() : defaultSigLevel), servers)) {
            throw std::runtime_error(std::format("Failed to register sync database {}: {}", section.GetName(), ALPM::GetError()));

        }
    }
}

auto Database::RegisterSyncDatabase(const std::string &treename, const std::bitset<32> &siglevelFlags, const std::vector<std::string> &servers) -> bool {
    int sigLevel = 0;

    // Since we have the same enum definition, just different names, it can just be used like so.
    Database db(alpm_register_syncdb(ALPM::GetHandle(), treename.c_str(), static_cast<alpm_siglevel_t>(siglevelFlags.to_ulong())));
    if (db.GetHandle() == nullptr) {
        return false;
    }

    db.SetServers(servers);

    return true;
}

auto Database::GetName() const -> std::string {
    return alpm_db_get_name(m_alpmdb);
}

auto Database::GetSigLevel() const -> int {
    return alpm_db_get_siglevel(m_alpmdb);
}

auto Database::IsValid() const -> std::optional<std::string> {
    if (alpm_db_get_valid(m_alpmdb) != 0) {
        return ALPM::GetError();
    }

    return {};
}

auto Database::GetPackage(const std::string &name) const -> Package {
    return Package(alpm_db_get_pkg(m_alpmdb, name.c_str()));
}

auto Database::Search(const std::string &expression) const -> std::vector<Package> {
    alpm_list_t *term = new alpm_list_t;
    term->prev = term;
    term->next = nullptr;
    term->data = new char[expression.size()];
    
    std::strcpy(static_cast<char*>(term->data), expression.c_str());

    alpm_list_t *list = nullptr;
    alpm_db_search(m_alpmdb, term, &list);

    std::vector<Package> results;
    for (size_t i = 0; i < alpm_list_count(list); i++) {
        results.emplace_back(static_cast<alpm_pkg_t*>(alpm_list_nth(list, i)->data));
    }

    return results;
}

auto Database::GetPackageCache() const -> std::vector<Package> {
    alpm_list_t *pkgCache = alpm_db_get_pkgcache(m_alpmdb);
    return Utils::ALPMListToVector<Package>(pkgCache);
}

auto Database::GetServers() const -> std::vector<std::string> {
    return Utils::ALPMListToVector<std::string>(alpm_db_get_servers(m_alpmdb));
}

auto Database::SetServers(const std::vector<std::string> &urls) -> bool {
    alpm_list_t *list = Utils::VectorToALPMList(urls);
    return alpm_db_set_servers(m_alpmdb, list);
    FREELIST(list);
}

auto Database::AddServer(const std::string &url) -> bool {
    return alpm_db_add_server(m_alpmdb, url.c_str());
}

auto Database::RemoveServer(const std::string &url) -> int {
    return alpm_db_remove_server(m_alpmdb, url.c_str());
}

auto Database::GetHandle() const -> alpm_db_t* {
    return m_alpmdb;
}

auto Database::MarkUpdate() const -> void {
    ALPM::GetCurrentTransaction()->AddDatabaseOperation(*this, Transaction::DatabaseOperation::Update);
}
