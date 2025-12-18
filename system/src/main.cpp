#include <iostream>

#include <argparse/argparse.hpp>

#include "Init.hpp"

auto main(int argc, char **argv) -> int {
    argparse::ArgumentParser arguments("system", "0.1");
    arguments.parse_args(argc, argv);

    Init init = Init("/dev/loop0", "5GiB", "10GiB");

    init.SetupPartitions();
    init.SetupFilesystems();
}
