#include "File.hpp"

#include "alpm.h"

using namespace ALPM;

File::File(alpm_file_t *file) :
    m_alpmFile(file) {}

auto File::GetMode() const -> mode_t {
    return m_alpmFile->mode;
}

auto File::GetName() const -> std::string {
    return m_alpmFile->name;
}

auto File::GetSize() const -> off_t {
    return m_alpmFile->size;
}

Backup::Backup(alpm_backup_t *backup) :
    m_alpmBackup(backup) {}

auto Backup::GetHash() const -> std::string {
    return m_alpmBackup->hash;
}

auto Backup::GetName() const -> std::string {
    return m_alpmBackup->name;
}
