#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <filesystem>
#include <type_traits>
#include <utility>
#include <bitset>

#include "Signature.hpp"

template <typename T, typename V>
concept SequenceContainer = requires (T container, typename T::value_type value) {
    typename T::value_type;
    typename T::reference;
    typename T::const_reference;
    typename T::iterator;
    typename T::const_iterator;
    typename T::difference_type;
    typename T::size_type;

    requires std::same_as<V, typename T::value_type>;

    { container.push_back(value) } -> std::same_as<decltype(container.push_back(value))>;
};


namespace ALPM {
    class Config {
        public:
            class Section {
                public:
                    class Value {
                        public:
                            inline explicit Value(const std::string &value) : m_value(value) {}
                            inline explicit Value(const Value &other) {
                                m_value = other.m_value;
                            }

                            inline explicit Value(Value &&other) {
                                m_value = std::exchange(other.m_value, "");
                            }

                            inline Value& operator=(const Value &other) {
                                if (this == &other) {
                                    return *this;
                                }

                                m_value = other.m_value;

                                return *this;
                            }

                            inline Value& operator=(const std::string &value) {
                                if (m_value == value) {
                                    return *this;
                                }

                                m_value = value;

                                return *this;
                            }

                            inline Value& operator=(Value &&other) {
                                if (this == &other) {
                                    return *this;
                                }

                                m_value = std::exchange(other.m_value, "");

                                return *this;
                            }

                            inline Value& operator=(std::string&& value) {
                                if (m_value == value) {
                                    return *this;
                                }

                                m_value = std::exchange(value, "");

                                return *this;
                            }

                            template <typename T> requires std::is_same_v<T, std::string>
                            inline auto As() const -> std::string {
                                return m_value;
                            }

                            template <typename T> requires std::is_integral_v<T>
                            inline auto As() const -> T {
                                auto value = T{};
                                auto [ptr, err] = std::from_chars(m_value.data(), m_value.data() + m_value.size(), value);
                                if (err == std::errc{}) {
                                    throw std::system_error(err, std::format("Failed to convert value to {}", typeid(T).name()));
                                }

                                return value;
                            }

                            template <typename T> requires SequenceContainer<T, std::string>
                            inline auto As() const -> T {
                                T container{};
                                std::stringstream ss(m_value);
                                std::string word;

                                while (ss >> word) {
                                    container.push_back(word);
                                }

                                return container;
                            }

                            template <typename T> requires SequenceContainer<T, Value>
                            inline auto As() const -> T {
                                T container{};
                                std::stringstream ss(m_value);
                                std::string word;
                                while (ss >> word) {
                                    container.push_back(Value(word));
                                }
                            }

                            template <typename T> requires std::is_same_v<T, Signature::Level>
                            inline auto As() const -> std::bitset<32> {
                                std::bitset<32> flags;
                                std::stringstream ss;
                                std::string word;
                                while (ss >> word) {
                                    if (word == "Never") {
                                        flags.reset();
                                    } else if (word == "Optional") {
                                        flags.reset(std::to_underlying(Signature::Level::PackageRequires));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseRequires));
                                        flags.set(std::to_underlying(Signature::Level::PackageOptional));
                                        flags.set(std::to_underlying(Signature::Level::DatabaseOptional));
                                    } else if (word == "Required") {
                                        flags.reset(std::to_underlying(Signature::Level::PackageOptional));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseOptional));
                                        flags.set(std::to_underlying(Signature::Level::PackageRequires));
                                        flags.set(std::to_underlying(Signature::Level::DatabaseRequires));
                                    } else if (word == "TrustedOnly") {
                                        flags.reset(std::to_underlying(Signature::Level::PackageUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::PackageMarginalOk));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseMarginalOk));
                                    } else if (word == "TrustAll") {
                                        flags.set(std::to_underlying(Signature::Level::PackageUnknownOk));
                                        flags.set(std::to_underlying(Signature::Level::PackageMarginalOk));
                                        flags.set(std::to_underlying(Signature::Level::DatabaseUnknownOk));
                                        flags.set(std::to_underlying(Signature::Level::DatabaseMarginalOk));
                                    } else if (word == "PackageNever") {
                                        flags.reset(std::to_underlying(Signature::Level::PackageUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::PackageOptional));
                                        flags.reset(std::to_underlying(Signature::Level::PackageUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::PackageMarginalOk));
                                    } else if (word == "PackageOptional") {
                                        flags.reset(std::to_underlying(Signature::Level::PackageRequires));
                                        flags.set(std::to_underlying(Signature::Level::PackageOptional));
                                    } else if (word == "PackageRequired") {
                                        flags.reset(std::to_underlying(Signature::Level::PackageOptional));
                                        flags.set(std::to_underlying(Signature::Level::PackageRequires));
                                    } else if (word == "PackageTrustedOnly") {
                                        flags.reset(std::to_underlying(Signature::Level::PackageUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::PackageMarginalOk));
                                    } else if (word == "PackageTrustAll") {
                                        flags.set(std::to_underlying(Signature::Level::PackageUnknownOk));
                                        flags.set(std::to_underlying(Signature::Level::PackageMarginalOk));
                                    } else if (word == "DatabaseNever") {
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseOptional));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseMarginalOk));
                                    } else if (word == "DatabaseOptional") {
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseRequires));
                                        flags.set(std::to_underlying(Signature::Level::DatabaseOptional));
                                    } else if (word == "DatabaseRequired") {
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseOptional));
                                        flags.set(std::to_underlying(Signature::Level::DatabaseRequires));
                                    } else if (word == "DatabaseTrustedOnly") {
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseUnknownOk));
                                        flags.reset(std::to_underlying(Signature::Level::DatabaseMarginalOk));
                                    } else if (word == "DatabaseTrustAll") {
                                        flags.set(std::to_underlying(Signature::Level::DatabaseUnknownOk));
                                        flags.set(std::to_underlying(Signature::Level::DatabaseMarginalOk));
                                    } else {
                                        throw std::runtime_error("Unknown configuration entry: " + word);
                                    }
                                }

                                return flags;
                            }

                        private:
                            std::string m_value;
                    };

                    Section(const std::string &name);

                    auto operator[](const std::string &option) const -> std::optional<const Value>;
                    auto operator[](const std::string &option) -> Value&;

                    auto GetName() const -> std::string;

                    auto GetOptions() const -> std::map<std::string, Value>;
                    auto GetOptions() -> std::map<std::string, Value>&;

                    auto GetOption(const std::string &option) const -> std::optional<const Value>;
                    auto GetOption(const std::string &option) -> Value&;

                    auto HasOption(const std::string &option) const -> bool;

                private:

                    std::string m_name;
                    std::map<std::string, Value> m_options;
            };

            explicit Config(const std::filesystem::path &configFile);
            explicit Config() = default;

            auto operator[](const std::string &sectionName) const -> std::optional<const Section>;
            auto operator[](const std::string &sectionName) -> Section;

            auto LoadFile(const std::filesystem::path &configGFile) -> void;

            auto GetSections() const -> std::vector<Section>;
            auto GetSections() -> std::vector<Section>&;

            auto GetSection(const std::string &sectionName) const -> std::optional<const Section>;
            auto GetSection(const std::string &sectionName) -> Section&;

        private:

            std::vector<Section> m_sections;
    };
}
