#pragma once

#include <cstdint>
#include <string>
#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include <sys/utsname.h>

#include <alpm.h>

namespace Utils {
    inline auto SizeToSectors(std::string size, uint32_t sectorSize) -> uint64_t {
        if (size.contains("KiB")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("KiB"))) * 1024) / sectorSize));
        } else if (size.contains("MiB")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("MiB"))) * std::pow(1024, 2)) / sectorSize));
        } else if (size.contains("GiB")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("GiB"))) * std::pow(1024, 3)) / sectorSize));
        } else if (size.contains("TiB")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("TiB"))) * std::pow(1024, 4)) / sectorSize));
        } else if (size.contains("K")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("K"))) * 1000) / sectorSize));
        } else if (size.contains("M")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("M"))) * std::pow(1000, 2)) / sectorSize));
        } else if (size.contains("G")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("G"))) * std::pow(1000, 3)) / sectorSize));
        } else if (size.contains("T")) {
            return static_cast<uint64_t>(std::ceil((std::stod(size.substr(0, size.find("T"))) * std::pow(1000, 4)) / sectorSize));
        } else {
            return 0;
        }
    }

    template <typename T>
    concept ALPMListConvertible = requires {
        typename T::UnderlyingType;
        requires requires (T CppObject) {
            { CppObject.GetHandle() } -> std::same_as<typename T::UnderlyingType*>;
        };
    };

    template <typename T>
    requires ALPMListConvertible<T>
    auto ALPMListToVector(const alpm_list_t *list) -> std::vector<T> {
        std::vector<T> vec;
        for (const alpm_list_t *i = list; i != nullptr; i = i->next) {
            vec.emplace_back(reinterpret_cast<T::UnderlyingType*>(i->data));
        }

        return vec;
    }
    template <typename T>
    requires std::same_as<T, std::string>
    auto ALPMListToVector(const alpm_list_t *list) -> std::vector<T> {
        std::vector<std::string> vec;
        for (const alpm_list_t *i = list; i != nullptr; i = i->next) {
            vec.push_back(std::string(reinterpret_cast<char*>(i->data)));
        }

        return vec;
    }

    template <typename T>
    requires ALPMListConvertible<T>
    auto VectorToALPMList(const std::vector<T> &vector) -> alpm_list_t* {
        alpm_list_t *list = nullptr;
        for (const T &element : vector) {
            list = alpm_list_add(list, element.GetHandle());
        }

        return list;
    }
    template <typename T>
    requires std::same_as<T, std::string>
    auto VectorToALPMList(const std::vector<T> &vector) -> alpm_list_t* {
        alpm_list_t *list = nullptr;
        for (const std::string &element : vector) {
            char *data = new char[element.size()];
            std::strcpy(data, element.c_str());
            list = alpm_list_add(list, data);
        }

        return list;
    }
    inline auto GetSystemArchitecture() -> std::string {
        struct utsname buffer;
        
        if (uname(&buffer) != 0) {
            return "unknown";
        }
        
        std::string machine = buffer.machine;
        
        // Normalize common architecture names
        if (machine == "x86_64" || machine == "amd64") {
            return "x86_64";
        } else if (machine == "aarch64" || machine == "arm64") {
            return "aarch64";
        } else if (machine == "i386" || machine == "i686") {
            return "i386";
        } else if (machine.find("arm") != std::string::npos) {
            return "arm";
        }
        
        return machine;
    }
}  // namespace Utils
