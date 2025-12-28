#include "Config.hpp"

#include <memory>
#include <fstream>

using namespace ALPM;

Config::Section::Section(const std::string &name) :
    m_name(name) {}

auto Config::Section::operator[](const std::string &option) const -> std::optional<const Value> {
    return GetOption(option);
}

auto Config::Section::operator[](const std::string &option) -> Value& {
    return GetOption(option);
}

auto Config::Section::GetName() const -> std::string {
    return m_name;
}

auto Config::Section::GetOptions() const -> std::map<std::string, Value> {
    return m_options;
}

auto Config::Section::GetOptions() -> std::map<std::string, Value>& {
    return m_options;
}

auto Config::Section::GetOption(const std::string &name) const -> std::optional<const Value> {
    if (m_options.contains(name)) {
        return std::make_optional<const Value>(m_options.at(name));
    }
    return {};
}

auto Config::Section::GetOption(const std::string &name) -> Value& {
    if (m_options.contains(name)) {
        return m_options.at(name);
    }

    m_options.emplace(name, Value(name));
    return m_options.at(name);
}

auto Config::Section::HasOption(const std::string &option) const -> bool {
    return m_options.contains(option);
}

Config::Config(const std::filesystem::path &configFile) :
    m_sections()
{
    LoadFile(configFile);
}

auto Config::LoadFile(const std::filesystem::path &configFile) -> void {
    if (!std::filesystem::exists(configFile)) {
        throw std::filesystem::filesystem_error("Failed to parse configuration file", std::make_error_code(std::errc::no_such_file_or_directory));
    }

    std::fstream file(configFile, std::fstream::in);

    std::string line;

    std::unique_ptr<Section> currentSection;
    while (std::getline(file, line)) {
        // Makes sure there are no spaces in front of the string
        line = line.substr(line.find_first_not_of(' ') == line.npos ? 0 : line.find_first_not_of(' '));
        if (line.starts_with('#')) {
            // Skip comments
            continue;
        }

        if (line.starts_with('[')) {
            std::string sectionName = line.substr(1).substr(0, line.substr(1).find_first_of(']'));
            if (currentSection.get()) {
                m_sections.push_back(*currentSection);
                currentSection = std::make_unique<Section>(sectionName);
            } else {
                currentSection = std::make_unique<Section>(sectionName);
            }

            continue;
        } else if (line.contains('=')) {
            // SomeKey = SomeValue # Some comment
            std::string key = line.substr(0, line.find_first_of('='));
            key = key.substr(0, key.find_last_not_of(' ') + 1);

            std::string value = line.substr(line.find_first_of('=') + 1);
            value = value.substr(value.find_first_not_of(' ') == value.npos ? 0 : value.find_first_not_of(' '));

            (*currentSection)[key] = value;
        }
    }
}

auto Config::operator[](const std::string &sectionName) const -> std::optional<const Section> {
    return GetSection(sectionName);
}

auto Config::operator[](const std::string &sectionName) -> Section {
    return GetSection(sectionName);
}

auto Config::GetSections() const -> std::vector<Section> {
    return m_sections;
}

auto Config::GetSections() -> std::vector<Section>& {
    return m_sections;
}

auto Config::GetSection(const std::string &sectionName) const -> std::optional<const Section> {
    for (const Section &section : m_sections) {
        if (section.GetName() == sectionName) {
            return section;
        }
    }

    return {};
}

auto Config::GetSection(const std::string &sectionName) -> Section& {
    for (Section &section : m_sections) {
        if (section.GetName() == sectionName) {
            return section;
        }
    }

    return m_sections.emplace_back(sectionName);
}

