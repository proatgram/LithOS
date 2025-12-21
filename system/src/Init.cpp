#include "Init.hpp"

#include <format>

#include <cstring>
#include <iostream>

#include <libfdisk/libfdisk.h>
#include <sys/mount.h>

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
    if (!m_diskConfiguration.ContainsPartition("boot") || !m_diskConfiguration.ContainsPartition("root")) {
        // We need at least a boot and root partition configuration
        throw std::runtime_error("Failed to setup partitions: Configuration doesn't contain a boot or root partition configuration.");
    }

    // Loop through each disk in the configuration, setting them up
    for (const DiskConfiguration::Disk &disk : m_diskConfiguration.GetDisks()) {
        fdisk_context *fdiskContext = fdisk_new_context();

        if (fdisk_assign_device(fdiskContext, disk.Name.c_str(), false) != 0) {
            fdisk_unref_context(fdiskContext);
            throw std::runtime_error(std::format("Failed to assign fdisk context to device: {}", disk.Name));
        }

        int rc;

        // Reset disk properties
        if ((rc = fdisk_reset_device_properties(fdiskContext)) != 0) {
            fdisk_unref_context(fdiskContext);
            throw std::runtime_error(std::format("Failed to reset device properties for device: {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }
        if ((rc = fdisk_reset_alignment(fdiskContext)) != 0) {
            fdisk_unref_context(fdiskContext);
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
            fdisk_unref_context(fdiskContext);
            throw std::runtime_error(std::format("Failed to create new disklabel for fdisk context for disk {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }
        uint32_t sectorSize = fdisk_get_sector_size(fdiskContext);
        
        // For now we just set the partition info, creating the filesystem will be later
        for (const std::shared_ptr<DiskConfiguration::Partition> &partition : disk.Partitions) {
            fdisk_partition *fdiskPartition = fdisk_new_partition();

            fdisk_partition_start_follow_default(fdiskPartition, 1); 
            fdisk_partition_partno_follow_default(fdiskPartition, 1);

            if ((rc = fdisk_partition_set_size(fdiskPartition, Utils::SizeToSectors(partition->Size, sectorSize))) != 0) {
                fdisk_unref_partition(fdiskPartition);
                fdisk_unref_context(fdiskContext);
                throw std::runtime_error(std::format("Failed to set partition size for disk {}, partition {}: {}: {}", disk.Name, partition->Name, std::strerror(-rc), rc));
            }
            if ((rc = fdisk_partition_set_name(fdiskPartition, partition->Label.c_str())) != 0) {
                fdisk_unref_partition(fdiskPartition);
                fdisk_unref_context(fdiskContext);
                throw std::runtime_error(std::format("Failed to set partition name for disk {}, partition {}: {}: {}", disk.Name, partition->Name, std::strerror(-rc), rc));
            }
            if (partition->Bootable) {
                if ((rc = fdisk_partition_set_attrs(fdiskPartition, "boot")) != 0) {
                    fdisk_unref_partition(fdiskPartition);
                    fdisk_unref_context(fdiskContext);
                    throw std::runtime_error(std::format("Failed to set boot partition as bootable for disk {}, partition {}: {}: {}", disk.Name, partition->Name, std::strerror(-rc), rc));
                }
            }

            fdisk_parttype *partitionType;
            switch (disk.Scheme) {
                case DiskConfiguration::PartitionTableScheme::GPT:
                        if ((partitionType = fdisk_label_get_parttype_from_string(fdiskLabel, partition->GPTGUID.c_str())) == nullptr) {
                            fdisk_unref_parttype(partitionType);
                            fdisk_unref_partition(fdiskPartition);
                            fdisk_unref_context(fdiskContext);
                            throw std::runtime_error(std::format("Failed to set partition type string for {} GUID = {}.", partition->Name, partition->GPTGUID));
                        }
                        
                    break;
                case DiskConfiguration::PartitionTableScheme::MBR:
                        if ((partitionType = fdisk_label_get_parttype_from_code(fdiskLabel, partition->MBRType)) == nullptr) {
                            fdisk_unref_parttype(partitionType);
                            fdisk_unref_partition(fdiskPartition);
                            fdisk_unref_context(fdiskContext);
                            throw std::runtime_error(std::format("Failed to set partition type number for {} code = {}", partition->Name, partition->MBRType));
                        }
                    break;
            }

            // Set partition type
            if ((rc = fdisk_partition_set_type(fdiskPartition, partitionType)) != 0) {
                fdisk_unref_parttype(partitionType);
                fdisk_unref_partition(fdiskPartition);
                fdisk_unref_context(fdiskContext);
                throw std::runtime_error(std::format("Failed to set partition type for disk {}, partition {}: {}: {}", disk.Name, partition->Name, std::strerror(-rc), rc));
            }
            fdisk_unref_parttype(partitionType);

            // Adds the partition to the context
            size_t partno;
            if ((rc = fdisk_add_partition(fdiskContext, fdiskPartition, &partno)) != 0) {
                fdisk_unref_partition(fdiskPartition);
                fdisk_unref_context(fdiskContext);
                throw std::runtime_error(std::format("Failed to add partition to fdisk context for disk {}, partition {}: {}: {}", disk.Name, partition->Name, std::strerror(-rc), rc));
            }

            fdisk_unref_partition(fdiskPartition);
            //partition->Order = partno;
        }


        // Applies everything
        if ((rc = fdisk_write_disklabel(fdiskContext)) != 0) {
            fdisk_unref_context(fdiskContext);
            throw std::runtime_error(std::format("Failed to write new disklabel for disk {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }

        // Updates/rereads partition table
        fdisk_reread_partition_table(fdiskContext);

        fdisk_deassign_device(fdiskContext, 1);
        fdisk_unref_context(fdiskContext);
    }
}

auto Init::SetupFilesystems() -> void {
    for (const DiskConfiguration::Disk &disk : m_diskConfiguration.GetDisks()) {
        fdisk_context *fdiskContext = fdisk_new_context();
        int rc{0};
        if ((rc = fdisk_assign_device(fdiskContext, disk.Name.c_str(), false)) != 0) {
            fdisk_unref_context(fdiskContext);
            throw std::runtime_error(std::format("Failed to assign device to fdisk context for disk {}: {}: {}", disk.Name, std::strerror(-rc), rc));
        }

        std::string diskPath = fdisk_get_devname(fdiskContext);

        fdisk_deassign_device(fdiskContext, false);
        fdisk_unref_context(fdiskContext);

        for (const std::shared_ptr<DiskConfiguration::Partition> &partition : disk.Partitions) {
            std::string partitionPath = diskPath;
            // For different format dev files
            if (diskPath.starts_with("/dev/loop") || diskPath.contains("/dev/mmcblk") || diskPath.starts_with("/dev/nvme")) {
                // partition->Order reassigned during partition creation to reflect fdisk partition numbering
                partitionPath += "p" + std::to_string(partition->Order);
            } else {
                partitionPath += std::to_string(partition->Order);
            }


            // To handle different FAT sizes
            if (partition->Filesystem.contains("vfat")) {
                // Just use FAT size of 32 for vfat since it's just glorifed FAT32
                rc = system(std::format("mkfs.vfat -F32 {}", partitionPath).c_str());

            } else if (partition->Filesystem.contains("fat")) {
                // Any other fat we determine the fat size based on the ending characters in the filesystem string, fat12, fat16, fat32
                std::string fatSize = partition->Filesystem.substr(partition->Filesystem.find("fat") + 3);
                rc = system(std::format("mkfs.fat -F{} {}", fatSize, partitionPath).c_str());
            } else if (partition->Filesystem == "btrfs") {
                rc = system(std::format("mkfs.btrfs -f {}", partitionPath).c_str());
            } else {
                rc = system(std::format("mkfs -t {} {}", partition->Filesystem, partitionPath).c_str());
            }

            if (rc != 0) {
                throw std::runtime_error(std::format("Failed to create filesystem {} on {}: Code {}", partition->Filesystem, partitionPath, WEXITSTATUS(rc)));
            }

            // Setup btrfs subvolumes if they exist
            if (partition->Filesystem == "btrfs") {
                std::filesystem::create_directories("/tmp/lithos/mnt");
                rc = mount(partitionPath.c_str(), "/tmp/lithos/mnt", "btrfs", 0, nullptr);
                if (rc != 0) {
                    std::filesystem::remove_all("/tmp/lithos");
                    throw std::runtime_error(std::format("Failed to mount partition {} on disk {} to create btrfs subvolumes: {}: {}", partition->Name, disk.Name, std::strerror(-rc), rc));
                }

                std::shared_ptr<DiskConfiguration::BtrfsPartition> btrfsPartition = std::reinterpret_pointer_cast<DiskConfiguration::BtrfsPartition>(partition);
                for (const DiskConfiguration::BtrFsSubvolume &subvolume : btrfsPartition->Subvolumes) {
                    rc = system(std::format("btrfs subvolume create /tmp/lithos/mnt/{}", subvolume.Subvolume).c_str());
                    if (rc != 0) {
                        rc = umount("/tmp/lithos/mnt");
                        if (rc != 0) {
                            std::filesystem::remove_all("/tmp/lithos");
                            throw std::runtime_error(std::format("Failed to create btrfs subvolume {} for disk {} on partition {} and failed to unmount.", subvolume.Subvolume, disk.Name, partition->Name));
                        }
                        std::filesystem::remove_all("/tmp/lithos");
                        throw std::runtime_error(std::format("Failed to create btrfs subvolume {} for disk {} on partition {}: {}: {}", subvolume.Subvolume, disk.Name, partition->Name, std::strerror(-rc), rc));
                    }
                }

                rc = umount("/tmp/lithos/mnt");
                if (rc != 0) {
                    std::filesystem::remove_all("/tmp/lithos");
                    throw std::runtime_error(std::format("Failed to unmount partition {} on disk {} to create btrfs subvolumes: {}: {}", partition->Name, disk.Name, std::strerror(-rc), rc));
                }
                std::filesystem::remove_all("/tmp/lithos");
            }
        }
    }

}
