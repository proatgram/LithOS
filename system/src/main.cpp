#include <iostream>

#include <argparse/argparse.hpp>

#include "Init.hpp"
#include "SystemConfiguration.hpp"

#include <iostream>

auto main(int argc, char **argv) -> int {
    argparse::ArgumentParser arguments("system", "0.1");
    arguments.parse_args(argc, argv);

}
