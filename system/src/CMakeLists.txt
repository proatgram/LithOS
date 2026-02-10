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
        system::ALPM
)

add_library(system_utils INTERFACE)
add_library(system::Utils ALIAS system_utils)

target_sources(system_utils
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include
        FILES ${CMAKE_SOURCE_DIR}/system/include/Utils.hpp
)

add_subdirectory("configs")
add_subdirectory("ALPM")
add_subdirectory("Status")
add_subdirectory("Event")
