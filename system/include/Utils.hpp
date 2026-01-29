#pragma once

#include <cstdint>
#include <string>
#include <cmath>
#include <vector>

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
    auto ALPMListToVector(const alpm_list_t* list) -> std::vector<T> {
        std::vector<T> vec;
        for (const alpm_list_t *i = list; i != nullptr; i = i->next) {
            vec.emplace_back(reinterpret_cast<T::UnderlyingType*>(i->data));
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
}  // namespace Utils
