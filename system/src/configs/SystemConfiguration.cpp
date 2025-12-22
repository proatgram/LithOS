#include "SystemConfiguration.hpp"

#include <yaml-cpp/yaml.h>
#include <format>
#include <ranges>
#include <iostream>

using namespace configs;

SystemConfiguration::SystemConfiguration(std::filesystem::path configurationFile) :
    SystemConfiguration(YAML::LoadFile(configurationFile.string()), configurationFile.parent_path()) {}

SystemConfiguration::SystemConfiguration(std::string configurationString) :
    SystemConfiguration(YAML::Load(configurationString)) {}


SystemConfiguration::SystemConfiguration(YAML::Node config, std::filesystem::path configPath) {
    // Handle inherits first
    if (config["inherits"]) {
        for (const YAML::Node &formula : config["inherits"]) {
            std::string inherit = formula.as<std::string>();
            std::filesystem::path formulaPath = (configPath.empty() ? std::filesystem::current_path() : configPath) /= inherit;

            if (inherit.ends_with(".yaml") || inherit.ends_with(".yml")) {
                if (!std::filesystem::exists(formulaPath)) {
                    throw std::runtime_error(std::format("Could not include formula {}, could not find at {}.", inherit, formulaPath.string()));
                }
            } else { // Doesn't have an extension in the config - check if a file exists with the default YAML extensions
                if (std::filesystem::exists(formulaPath.string() += ".yaml")) {
                    formulaPath += ".yaml";
                } else if (std::filesystem::exists(formulaPath.string() += ".yml")) {
                    formulaPath += ".yml";
                } else {
                    throw std::runtime_error(std::format("Could not include formula {}, could not find at {} with extension yaml or yml.", inherit, formulaPath.string()));
                }
            }

            m_inherits.emplace_back(formulaPath);
        }
    }

    if (config["config"]) {
        config = config["config"];

        // Handle config:system
        if (config["system"]) {
            System system;
            system.Hostname = config["system"]["hostname"].as<std::string>("lithos");
            system.Locale = config["system"]["locale"].as<std::string>("en_US.UTF-8");
            system.Keymap = config["system"]["keymap"].as<std::string>("us");
            
            if (config["system"]["users"]) {
                for (auto it = std::begin(config["system"]["users"]); it != std::end(config["system"]["users"]); it++) {
                    YAML::Node yamlUser = it->second;
                    User user;
                    user.Username = it->first.as<std::string>();
                    user.Fullname = yamlUser["fullname"].as<std::string>("");
                    user.Shell = yamlUser["shell"].as<std::string>("/bin/bash");
                    
                    if (yamlUser["groups"]) {
                        for (const YAML::Node &group : yamlUser["groups"]) {
                            user.Groups.push_back(group.as<std::string>());
                        }
                    }
                }
            }
            m_configEntries.push_back(system);
        }

        // Handle config:packages
        if (config["packages"]) {
            std::vector<Package> packages;
            for (auto it = std::cbegin(config["packages"]); it != std::end(config["packages"]); it++) {
                for (const YAML::Node &package : it->second) {
                    packages.push_back(Package{.Name = package.as<std::string>(), .Category = it->first.as<std::string>()});
                }
            }
            m_configEntries.push_back(packages);
        }

        // Handle config:services
        if (config["services"]) {
            std::vector<Service> services;
            for (auto it = std::cbegin(config["services"]); it != std::end(config["services"]); it++) {
                for (const YAML::Node &service : it->second) {
                    services.push_back(Service{.Name = service.as<std::string>(), .Category = it->first.as<std::string>()});
                }
            }
            m_configEntries.push_back(services);
        }

        // Handle config:files
        if (config["files"]) {
            std::vector<File> files;
            for (auto it = std::begin(config["files"]); it != std::end(config["files"]); std::advance(it, 1)) {
                YAML::Node yamlFile = it->second;
                File file;

                file.Path = it->first.as<std::string>();
                file.Contents = yamlFile["contents"].as<std::string>();
                
                std::string mode = yamlFile["mode"].as<std::string>();

                if (mode == "replace") {
                    file.Mode = File::FileMode::Replace;
                    File::ReplaceOptions replaceOptions;

                    replaceOptions.What = yamlFile["replace"].as<std::string>();

                    file.Options = replaceOptions;
                } else if (mode == "insert") {
                    file.Mode = File::FileMode::Insert;
                    File::InsertOptions insertOptions;

                    // Makes sure that at least one is defined, but not both
                    if (!(yamlFile["after"].IsDefined() ^ yamlFile["before"].IsDefined())) {
                        throw std::runtime_error("Invalid file insert options, requires either after or before and not both.");
                    }

                    insertOptions.After = yamlFile["after"].as<std::string>("");
                    insertOptions.Before = yamlFile["before"].as<std::string>("");

                    file.Options = insertOptions;
                } else if (mode == "create") {
                    file.Mode = File::FileMode::Create;
                    File::CreateOptions createOptions;

                    createOptions.Replace = yamlFile["replace"].as<bool>(false);
                    createOptions.Permissions = yamlFile["permissions"].as<std::string>("255");

                    file.Options = createOptions;
                } else {
                    throw std::runtime_error(std::format("No such file mode {}", mode));
                }

                files.push_back(file);
            }
            m_configEntries.push_back(files);
        }

    } else  {
        throw std::runtime_error("Bad formula format: No \"config\" section.");
    }
}

auto SystemConfiguration::GetFiles() const -> std::vector<File> {
    std::vector<File> files;

    auto it = std::find_if(m_configEntries.cbegin(), m_configEntries.cend(), [](const std::variant<std::vector<Package>, std::vector<Service>, std::vector<File>, System> &variant) -> bool {
        return std::holds_alternative<std::vector<File>>(variant);
    });

    if (it != m_configEntries.end()) {
        files.append_range(std::get<std::vector<File>>(*it));
    }

    for (const SystemConfiguration &inherit : m_inherits) {
        files.append_range(inherit.GetFiles());
    }

    return files;
}

auto SystemConfiguration::GetPackages() const -> std::vector<Package> {
    std::vector<Package> packages;

    auto it = std::find_if(m_configEntries.cbegin(), m_configEntries.cend(), [](const std::variant<std::vector<Package>, std::vector<Service>, std::vector<File>, System> &variant) -> bool {
        return std::holds_alternative<std::vector<Package>>(variant);
    });

    if (it != m_configEntries.end()) {
        packages.append_range(std::get<std::vector<Package>>(*it));
    }

    for (const SystemConfiguration &inherit : m_inherits) {
        packages.append_range(inherit.GetPackages());
    }

    return packages;
}

auto SystemConfiguration::GetServices() const -> std::vector<Service> {
    std::vector<Service> services;

    auto it = std::find_if(m_configEntries.cbegin(), m_configEntries.cend(), [](const std::variant<std::vector<Package>, std::vector<Service>, std::vector<File>, System> &variant) -> bool {
        return std::holds_alternative<std::vector<Service>>(variant);
    });

    if (it != m_configEntries.end()) {
        services.append_range(std::get<std::vector<Service>>(*it));
    }

    for (const SystemConfiguration &inherit : m_inherits) {
        services.append_range(inherit.GetServices());
    }

    return services;
}

auto SystemConfiguration::GetSystemConfig() const -> std::optional<System> {
    std::optional<System> system;

    auto it = std::find_if(m_configEntries.cbegin(), m_configEntries.cend(), [](const std::variant<std::vector<Package>, std::vector<Service>, std::vector<File>, System> &variant) -> bool {
        return std::holds_alternative<System>(variant);
    });
    
    if (it != m_configEntries.end()) {
        system = std::get<System>(*it);

        // Check to make sure that any inherited formulae don't also have a system config,
        // as that would be ambiguous.
        for (const SystemConfiguration &inherit : m_inherits) {
            if (inherit.GetSystemConfig().has_value()) {
                throw std::runtime_error("Invalid configuration: Multiple \"system\" sections are ambiguous.");
            }
        }
    } else {
        for (const SystemConfiguration &inherit : m_inherits) {
            if (inherit.GetSystemConfig().has_value()) {
                // Check to make sure that any inherited formulae don't also have a system config,
                // as that would be ambiguous.
                if (system.has_value()) {
                    throw std::runtime_error("Invalid configuration: Multiple \"system\" sections are ambiguous.");
                }
                system = inherit.GetSystemConfig();
            }
        }
    }

    return system;
}
