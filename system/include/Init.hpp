#pragma once

#include <filesystem>
#include <string_view>
#include <string>
#include <optional>
#include <map>

#include "DiskConfiguration.hpp"

struct fdisk_context;

class Init {
    public:
        Init(std::filesystem::path devicePath, std::string rootSize, std::string homeSize);
        Init(configs::DiskConfiguration diskConfiguration, std::optional<std::map<std::string, std::string>> aliasMappings = std::nullopt);

        auto SetupPartitions() -> void;
        auto SetupFilesystems() -> void;

    private:
        inline static constexpr std::string_view DEFAULT_DISK_CONFIGURATION = R"EOF(
disks:
  - path: "{}"
    partitioning:
      scheme: gpt
      partitions:
        boot:
          label: BOOT
          filesystem: vfat
          guid: C12A7328-F81F-11D2-BA4B-00A0C93EC93B
          size: 512MiB
          mountpoint: "/boot/efi"
          order: 1
        root:
          size: {}
          order: 2
        home:
          label: HOME
          guid: 933AC7E1-2EB4-4F13-B844-0E14E2AEF915
          size: {}
          mountpoint: /home
          order: 3
        )EOF";

        configs::DiskConfiguration m_diskConfiguration;
};
