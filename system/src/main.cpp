#include <argparse/argparse.hpp>

#include "ALPM.hpp"

auto main(int argc, char **argv) -> int {
    argparse::ArgumentParser arguments("system", "0.1");
    arguments.parse_args(argc, argv);

    ALPM::ALPM::Initialize();

    ALPM::ALPM::GetCurrentTransaction()->AddSystemUpgradeOperation();
    ALPM::ALPM::GetCurrentTransaction()->Apply();
}
