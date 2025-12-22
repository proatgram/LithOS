add_library(system_init)
add_library(system::init ALIAS system_init)

target_sources(system_init
    PUBLIC Init.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include
    FILES ${CMAKE_SOURCE_DIR}/system/include/Init.hpp
)

target_link_libraries(system_init
    PUBLIC
        PkgConfig::libfdisk
        system::configs::DiskConfiguration
        system::configs::SystemConfiguration
)

add_subdirectory("configs")
