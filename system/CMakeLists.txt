add_executable(system)

target_sources(system
    PRIVATE "src/main.cpp"
)

target_link_libraries(system
    PRIVATE
        argparse::argparse
        system::init
        system::ALPM
        system::Event
        system::PosixSignals
)

add_subdirectory("src")
