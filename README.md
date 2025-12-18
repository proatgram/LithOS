# LithOS - Flexible, deterministic, customizable, and stable Immutable and Transactional Linux

Utilizing btrfs technology with a base of Arch Linux, we are able to guarentee stability while also allowing for a deterministic system every boot that can be easily configured to your liking.

## How it works

The partition scheme is as follows:

```
/
/@system
/@build
/@revisions
```

The current root is contained in /@system and overlayed ontop of / as read-only. /@revisions contains the different revisions of your root system, and can be restored to /@system whenever you need. /@build is used as a working directory for building a new rootfs. You also have the ability to select between multiple different revisions at boot time in the bootloader menu.


You can make changes to the root during runtime using the system command, and test those changes or use them temporarily during runtime. Upon applying them using `system apply`, a new rootfs image will be built and saved as the latest revision, and will be used at the next boot cycle. If you don't like the changes you made, you can simply reboot without applying, and it will be like you never made those changes at all. This is the motivation behind this distro - The ability to make changes easily and quickly, and to be able to have those changes immediately in a predictable or easily revertable manner. This is what differs from most immutable and atomic distros, you always have to rebuild and then boot before using it.


You can also specify system configurations in the /etc/system.conf.d/ directory. Each Formula, or LithOS formated YAML file in the directory will be used to create a predictable and atomic image. The `system` command will modify theses files for you, but for people who want to save their system configuration, or do a bulk configuration, this is for you.

## How to install

Currently, you can install it using the `system` tool. You can specify `system init /dev/sd...` to bootstrap a new LithOS system. Additionally, you can give it configuration files to use when bootstrapping, and an additional install.yaml which can help determine partition scheming for the new system as well.
