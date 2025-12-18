find_program(PACSTRAP pacstrap REQUIRED)

add_executable(system)

target_sources(system
    PRIVATE "src/main.cpp"
)

target_link_libraries(system
    PRIVATE
        argparse::argparse
        system::init
)

add_subdirectory("src")
