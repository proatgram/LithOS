#pragma once

#include <filesystem>
#include <vector>
#include <variant>
#include <optional>

namespace YAML {
    class Node;
}

namespace configs {
    class SystemConfiguration {
        public:
            struct Package {
                std::string Name;
                std::string Category;
            };

            struct Service {
                std::string Name;
                std::string Category;
            };

            struct File {
                enum class FileMode {
                    Replace,
                    Create,
                    Insert
                };

                struct ReplaceOptions {
                    std::string What;
                };

                struct CreateOptions {
                    bool Replace;
                    std::string Permissions;
                };

                struct InsertOptions {
                    std::string After;
                    std::string Before;
                };

                std::filesystem::path Path;
                FileMode Mode;
                std::string Contents;

                std::variant<std::monostate, ReplaceOptions, CreateOptions, InsertOptions> Options;
            };

            struct User {
                std::string Username;
                std::string Fullname;
                std::vector<std::string> Groups;
                std::string Shell;
            };

            struct System {
                std::string Hostname;
                std::string Timezone;
                std::string Locale;
                std::string Keymap;
                std::vector<User> Users;
            };

            SystemConfiguration(std::filesystem::path configurationFile);
            SystemConfiguration(std::string configurationString);

            auto GetFiles() const -> std::vector<File>;
            auto GetPackages() const -> std::vector<Package>;
            auto GetServices() const -> std::vector<Service>;
            auto GetSystemConfig() const -> std::optional<System>;
            
        private:
            SystemConfiguration(YAML::Node config, std::filesystem::path configPath = {});
            
            std::vector<SystemConfiguration> m_inherits;
            std::vector<std::variant<std::vector<Package>, std::vector<Service>, std::vector<File>, System>> m_configEntries;
    };
}  // configs
