#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace YAML {
    class Node;
}

namespace configs {
    class DiskConfiguration {
        public:
            enum class PartitionTableScheme {
                GPT,
                MBR
            };

            struct Partition {
                std::string Label;
                std::string Name;
                std::string Filesystem;
                uint32_t MBRType;
                std::string GPTGUID;
                std::string Size;
                std::string Mountpoint;
                uint8_t Order;
                bool Bootable;
            };

            struct BtrFsSubvolume {
                std::string Subvolume;
                std::string Mountpoint;
                std::string Type;
            };

            struct BtrfsPartition : public Partition {
                std::vector<BtrFsSubvolume> Subvolumes;
            };

            struct Disk {
                std::string Name;
                bool IsAlias;
                PartitionTableScheme Scheme;
                std::vector<std::shared_ptr<Partition>> Partitions;
            };

            explicit DiskConfiguration(std::filesystem::path configurationFile);
            explicit DiskConfiguration(std::string yamlConfigurationFile);

            DiskConfiguration() = default;

            auto GetDisks() const -> std::vector<Disk>;
            auto ApplyAliases(std::map<std::string, std::string> aliasesMap) -> DiskConfiguration&;
            auto ContainsPartition(const std::string &name) -> bool;
            auto GetPartitionsWithFilesystem(const std::string &filesystem) -> std::vector<std::shared_ptr<Partition>>;

        private:
            explicit DiskConfiguration(YAML::Node config);
            std::vector<Disk> m_disks;
    };
}  // namespace configs
