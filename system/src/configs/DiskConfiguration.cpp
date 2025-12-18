#include "DiskConfiguration.hpp"

#include <yaml-cpp/yaml.h>

using namespace configs;

DiskConfiguration::DiskConfiguration(std::filesystem::path configurationFile) :
    DiskConfiguration(YAML::LoadFile(configurationFile.string())["disks"])
{

}

DiskConfiguration::DiskConfiguration(std::string yamlConfigurationString) :
    DiskConfiguration(YAML::Load(yamlConfigurationString)["disks"])
{

}

DiskConfiguration::DiskConfiguration(YAML::Node config) {
    if (!config.IsDefined()) {
        throw std::runtime_error("Invalid partition configuration file: Could not find root node \"disks\"");
    }

    for (const YAML::Node disk : config) {
        Disk diskConfig;

        // Determine if entry is a disk alias (set through command-line) or a direct path.
        if (disk["alias"]) {
            diskConfig.IsAlias = true;
            diskConfig.Name = disk["alias"].as<std::string>();
        } else if (disk["path"]) {
            diskConfig.IsAlias = false;
            diskConfig.Name = disk["path"].as<std::string>();
        } else {
            throw std::runtime_error("Invalid disk configuration entry found.");
        }

        YAML::Node partitioning = disk["partitioning"];
        if (!partitioning || partitioning.size() == 0) {
            throw std::runtime_error("Invalid disk configuration: No \"Partitioning\" entry found or is empty.");
        }
        
        // Set partition scheme.
        if (partitioning["scheme"]) {
            std::string scheme = partitioning["scheme"].as<std::string>();
            if (scheme == "gpt" || scheme == "GPT") {
                diskConfig.Scheme = PartitionTableScheme::GPT;
            } else if (scheme == "mbr" || scheme == "MBR") {
                diskConfig.Scheme = PartitionTableScheme::MBR;
            } else {
                throw std::runtime_error("Invalid disk partition scheme: " + scheme);
            }
        } else {
            diskConfig.Scheme = PartitionTableScheme::GPT;
        }

        // Set partitions
        if (!partitioning["partitions"] || partitioning["partitions"].size() == 0) {
            throw std::runtime_error("Invalid disk configuration: No \"partitions\" entry found or is empty.");
        }

        // Get maximum partition order for later, if it exists.
        uint8_t partitioningMaxOrder{0};
        for (YAML::const_iterator it = partitioning["partitions"].begin(); it != partitioning["partitions"].end(); std::advance(it, 1)) {
            YAML::Node partitionProperties = it->second;
            if (partitionProperties["order"] && partitionProperties["order"].as<uint8_t>() > partitioningMaxOrder) {
                partitioningMaxOrder = partitionProperties["order"].as<uint8_t>();
            } else {
                // If there isn't an order, then there won't be any.
                break;
            }
        }

        for (YAML::const_iterator it = partitioning["partitions"].begin(); it != partitioning["partitions"].end(); std::advance(it, 1)) {
            std::shared_ptr<Partition> partitionConfig;

            std::string partitionName = it->first.as<std::string>();
            YAML::Node partitionProperties = it->second;

            // Partition order
            uint8_t partitionOrder{0};
            std::string partitionLabel;
            std::string partitionFilesystem;
            std::string partitionMountpoint;
            std::string partitionSize;
            uint32_t partitionMBRType{0};
            std::string partitionGPTGUID;

            if (partitionProperties["order"]) {
                partitionOrder = partitionProperties["order"].as<uint8_t>();
            } else if (partitioning["partitions"].size() > 1) {
                throw std::runtime_error("Invalid disk partition configuration: Must have attribute \"order\" when specifying more than one partition.");
            } // Order optional unless number of partitions is > 1

            if (partitionProperties["label"]) {
                partitionLabel = partitionProperties["label"].as<std::string>();
            } // Label optional

            if (partitionProperties["filesystem"]) {
                partitionFilesystem = partitionProperties["filesystem"].as<std::string>();
            } else  {
                partitionFilesystem = "ext4";
            } // Filesystem optional, defaults to ext4

            if (partitionProperties["mountpoint"]) {
                partitionMountpoint = partitionProperties["mountpoint"].as<std::string>();
            } // Mountpoint optional

            if (partitionProperties["size"]) {
                partitionSize = partitionProperties["size"].as<std::string>();
            } else if (partitioning["partitions"].size() > 1 && partitionOrder != partitioningMaxOrder) {
                throw std::runtime_error("Invalid disk partition configuration: Must have attribute \"size\" when specifying more than one partiton that isn't the final partition.");
            } // Size optional unless it's not the 

            if (partitionProperties["guid"]) {
                partitionGPTGUID = partitionProperties["guid"].as<std::string>();
            } else {
                partitionGPTGUID = "0FC63DAF-8483-4772-8E79-3D69D8477DE4";
            } // GPT GUID optional, defaults to Linux filesystem data

            if (partitionProperties["type"]) {
                partitionMBRType = partitionProperties["type"].as<uint32_t>();
            } else {
                partitionMBRType = 0x83;
            } // MBR Type optional, defaults to Linux filesystem data

            if (partitionName == "root") {
                // Handle implicit root configuration
                std::shared_ptr<BtrfsPartition> rootPartitionConfig = std::make_shared<BtrfsPartition>();

                rootPartitionConfig->Filesystem = "btrfs";
                rootPartitionConfig->Label = (partitionLabel.empty() ? "ROOT" : partitionLabel);
                rootPartitionConfig->Mountpoint = "/";
                rootPartitionConfig->Order = partitionOrder;

                rootPartitionConfig->Subvolumes.push_back({.Subvolume = "/@system", .Mountpoint = "/system"});
                rootPartitionConfig->Subvolumes.push_back({.Subvolume = "/@revisions", .Mountpoint = "/revisions"});

                rootPartitionConfig->Size = partitionSize;
                
                if (partitioning["scheme"].as<std::string>() == "gpt") {
                    rootPartitionConfig->GPTGUID = "4F68BCE3-E8CD-4DB1-96E7-FBCAF984B709";
                } else if (partitioning["scheme"].as<std::string>() == "mbr") {
                    rootPartitionConfig->MBRType = 0x83;
                }


                partitionConfig = rootPartitionConfig;
            } else  {
                if (partitionFilesystem == "btrfs") {
                    // Btrfs partition type (special features like subvolumes)
                    std::shared_ptr<BtrfsPartition> btrfsPartitionConfig = std::make_shared<BtrfsPartition>();

                    btrfsPartitionConfig->Filesystem = "btrfs";
                    btrfsPartitionConfig->Label = partitionLabel;
                    btrfsPartitionConfig->Mountpoint = partitionMountpoint;
                    btrfsPartitionConfig->Size = partitionSize;
                    btrfsPartitionConfig->GPTGUID = partitionGPTGUID;
                    btrfsPartitionConfig->MBRType = partitionMBRType;

                    // Handling for btrfs subvolume configuration
                    if (partitionProperties["subvolumes"]) {
                        for (const YAML::Node subvolume : partitionProperties["subvolumes"]) {
                            BtrFsSubvolume subvolumeConfig;
                            if (subvolume["volume"]) {
                                subvolumeConfig.Subvolume = subvolume["volume"].as<std::string>();
                            } else {
                                throw std::runtime_error("Invalid disk partition configuration: btrfs partition subvolume must have a volume attribute.");
                            }

                            if (subvolume["mountpoint"]) {
                                subvolumeConfig.Mountpoint = subvolume["mountpoint"].as<std::string>();
                            } // Mountpoint optional

                            btrfsPartitionConfig->Subvolumes.push_back(subvolumeConfig);
                        }
                    }

                    partitionConfig = btrfsPartitionConfig;

                } else {
                    // Normal partition type (no special features like btrfs)
                    partitionConfig = std::make_shared<Partition>();

                    partitionConfig->Filesystem = partitionFilesystem;
                    partitionConfig->Label = partitionLabel;
                    partitionConfig->Size = partitionSize;
                    partitionConfig->Order = partitionOrder;
                    partitionConfig->Mountpoint = partitionMountpoint;
                    partitionConfig->GPTGUID = partitionGPTGUID;
                    partitionConfig->MBRType = partitionMBRType;
                }
            }

            diskConfig.Partitions.push_back(partitionConfig);
        }
        
        m_disks.push_back(diskConfig);
    }
}

auto DiskConfiguration::GetDisks() const -> std::vector<Disk> {
    return m_disks;
}

auto DiskConfiguration::ApplyAliases(std::map<std::string, std::string> aliasesMap) -> DiskConfiguration& {
    for (const auto &[alias, key] : aliasesMap) {
        for (Disk &disk : m_disks) {
            if (disk.IsAlias && disk.Name == alias) {
                disk.Name = key;
            }
        }
    }

    return *this;
}
