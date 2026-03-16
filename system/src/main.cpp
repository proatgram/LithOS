#include <argparse/argparse.hpp>
#include "PosixSignals.hpp"

#include "ALPM.hpp"

auto main(int argc, char **argv) -> int {
    argparse::ArgumentParser arguments("system", "0.1");
    arguments.parse_args(argc, argv);

    ALPM::ALPM::Initialize();
    POSIXSignals::Signal::InitHandlers();

    for (const ALPM::Database &db : ALPM::ALPM::GetSyncDatabases()) {
        db.MarkUpdate();
    }

    ALPM::ALPM::GetCurrentTransaction()->SetFlags(ALPM::Transaction::OperationFlags::ForceDatabase);
    ALPM::ALPM::GetCurrentTransaction()->AddSystemUpgradeOperation();
    ALPM::ALPM::GetCurrentTransaction()->Apply();
}
