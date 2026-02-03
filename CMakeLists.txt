cmake_minimum_required(VERSION 3.28)

project(LithOS
    VERSION 0.1
    DESCRIPTION "LithOS - A stone like Linux distrobution made for stability and customizability."
    HOMEPAGE_URL "https://github.com/proatgram/LithOS"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

find_package(argparse REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(indicators REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(libalpm REQUIRED IMPORTED_TARGET GLOBAL libalpm)
pkg_check_modules(libfdisk REQUIRED IMPORTED_TARGET GLOBAL fdisk)

add_subdirectory("system")
