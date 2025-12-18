#include "Init.hpp"

#include <format>

#include <cstring>
#include <iostream>
#include <libfdisk/libfdisk.h>

#include "Utils.hpp"

using namespace configs;

Init::Init(std::filesystem::path devicePath, std::string rootSize, std::string homeSize) {
    m_diskConfiguration = DiskConfiguration(std::format(DEFAULT_DISK_CONFIGURATION, devicePath.c_str(), rootSize, homeSize));
}

Init::Init(DiskConfiguration diskConfiguration, std::optional<std::map<std::string, std::string>> aliasMappings) {
    if (aliasMappings.has_value()) {
        m_diskConfiguration = diskConfiguration.ApplyAliases(aliasMappings.value());
    } else {
        m_diskConfiguration = diskConfiguration;
    }
}

auto Init::SetupPartitions() -> void {
    for (const DiskConfiguration::Disk &disk : m_diskConfiguration.GetDisks()) {
        fdisk_context *fdiskContext = fdisk_new_context();
        if (fdisk_assign_device(fdiskContext, disk.Name.c_str(), false) != 0) {
            throw std::runtime_error(std::format("Failed to assign fdisk context to device: {}", disk.Name));
        }

        int rc;
        if ((rc = fdisk_reset_device_properties(fdiskContext)) != 0) {
            throw std::runtime_error(std::format("Failed to reset device properties for device: {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }

        if ((rc = fdisk_reset_alignment(fdiskContext)) != 0) {
            throw std::runtime_error(std::format("Failed to reset alignment for device: {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }

        // Set partition table type
        fdisk_label *fdiskLabel;
        switch (disk.Scheme) {
            case DiskConfiguration::PartitionTableScheme::GPT:
                    rc = fdisk_create_disklabel(fdiskContext, "gpt");
                    fdiskLabel = fdisk_get_label(fdiskContext, nullptr);
                break;
            case DiskConfiguration::PartitionTableScheme::MBR:
                    rc = fdisk_create_disklabel(fdiskContext, "dos");
                    fdiskLabel = fdisk_get_label(fdiskContext, nullptr);
                break;
            default:
                throw std::runtime_error("Invalid partition scheme caught.");
                break;
        }
        if (rc != 0 || fdiskLabel == nullptr) {
            throw std::runtime_error(std::format("Failed to create new disklabel for fdisk context for disk {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }
        uint32_t sectorSize = fdisk_get_sector_size(fdiskContext);
        
        // For now we just set the size and type, filesystem will be later
        for (const std::shared_ptr<DiskConfiguration::Partition> &partition : disk.Partitions) {
            fdisk_partition *fdiskPartition = fdisk_new_partition();

            fdisk_partition_start_follow_default(fdiskPartition, 1); 
            fdisk_partition_partno_follow_default(fdiskPartition, 1);

            if ((rc = fdisk_partition_set_size(fdiskPartition, Utils::SizeToSectors(partition->Size, sectorSize))) != 0) {
                fdisk_unref_partition(fdiskPartition);
                throw std::runtime_error(std::format("Failed to set partition size for disk {}, partition {}: {}: {}", disk.Name, partition->Label, std::strerror(-rc), rc));
            }
            if ((rc = fdisk_partition_set_name(fdiskPartition, partition->Label.c_str())) != 0) {
                fdisk_unref_partition(fdiskPartition);
                throw std::runtime_error(std::format("Failed to set partition name for disk {}, partition {}: {}: {}", disk.Name, partition->Label, std::strerror(-rc), rc));
            }
            if (partition->Bootable) {
                // TODO: DOS/MBR boot flag (not that big of a priority cuz who would be using DOS/MBR these days lmfao)
            }

            fdisk_parttype *partitionType;
            switch (disk.Scheme) {
                case DiskConfiguration::PartitionTableScheme::GPT:
                        if ((partitionType = fdisk_label_get_parttype_from_string(fdiskLabel, partition->GPTGUID.c_str())) == nullptr) {
                            fdisk_unref_parttype(partitionType);
                            fdisk_unref_partition(fdiskPartition);
                            throw std::runtime_error(std::format("Failed to set partition type string for {} GUID = {}.", partition->Label, partition->GPTGUID));
                        }
                        
                    break;
                case DiskConfiguration::PartitionTableScheme::MBR:
                        if ((partitionType = fdisk_label_get_parttype_from_code(fdiskLabel, partition->MBRType)) == nullptr) {
                            fdisk_unref_parttype(partitionType);
                            fdisk_unref_partition(fdiskPartition);
                            throw std::runtime_error(std::format("Failed to set partition type number for {} code = {}", partition->Label, partition->MBRType));
                        }
                    break;
            }

            if ((rc = fdisk_partition_set_type(fdiskPartition, partitionType)) != 0) {
                fdisk_unref_parttype(partitionType);
                fdisk_unref_partition(fdiskPartition);
                throw std::runtime_error(std::format("Failed to set partition type for disk {}, partition {}: {}: {}", disk.Name, partition->Label, std::strerror(-rc), rc));
            }
            fdisk_unref_parttype(partitionType);

            size_t partno;
            if ((rc = fdisk_add_partition(fdiskContext, fdiskPartition, &partno)) != 0) {
                fdisk_unref_partition(fdiskPartition);
                throw std::runtime_error(std::format("Failed to add partition to fdisk context for disk {}, partition {}: {}: {}", disk.Name, partition->Label, std::strerror(-rc), rc));
            }

            fdisk_unref_partition(fdiskPartition);
        }


        // Applies everything
        if ((rc = fdisk_write_disklabel(fdiskContext)) != 0) {
            throw std::runtime_error(std::format("Failed to write new disklabel for disk {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }

        fdisk_reread_partition_table(fdiskContext);

        fdisk_deassign_device(fdiskContext, 1);
        fdisk_unref_context(fdiskContext);
    }
}
