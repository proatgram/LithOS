#include <argparse/argparse.hpp>

#include "ALPM.hpp"

#include <iostream>

auto main(int argc, char **argv) -> int {
    argparse::ArgumentParser arguments("system", "0.1");
    arguments.parse_args(argc, argv);

    ALPM::ALPM::Initialize();

    /*
    for (const ALPM::Database &database : ALPM::ALPM::GetSyncDatabases()) {
        std::cout << database.GetName() << std::endl;
        for (const ALPM::Package &package : database.Search("")) {
            std::cout << "  -> " << package.GetName() << std::endl;
        }
    }
    */
}
