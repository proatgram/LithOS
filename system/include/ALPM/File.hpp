#pragma once

#include <sys/types.h>

#include <string>

struct _alpm_file_t;
typedef _alpm_file_t alpm_file_t;
struct _alpm_backup_t;
typedef _alpm_backup_t alpm_backup_t;

namespace ALPM {
    class File {
        public:
            explicit File(alpm_file_t *file);

            auto GetMode() const -> mode_t;
            auto GetName() const -> std::string;
            auto GetSize() const -> off_t;

        private:
            alpm_file_t *m_alpmFile;
    };

    class Backup {
        public:

            explicit Backup(alpm_backup_t *backup);

            auto GetHash() const -> std::string;
            auto GetName() const -> std::string;
        private:

            alpm_backup_t *m_alpmBackup;
    };
}  // namespace ALPM
