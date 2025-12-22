add_library(system_configs_DiskConfiguration)
add_library(system::configs::DiskConfiguration ALIAS system_configs_DiskConfiguration)

target_sources(system_configs_DiskConfiguration
    PUBLIC DiskConfiguration.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/configs
    FILES ${CMAKE_SOURCE_DIR}/system/include/configs/DiskConfiguration.hpp
)

target_link_libraries(system_configs_DiskConfiguration
    PUBLIC
        yaml-cpp::yaml-cpp
)

add_library(system_configs_SystemConfiguration)
add_library(system::configs::SystemConfiguration ALIAS system_configs_SystemConfiguration)

target_sources(system_configs_SystemConfiguration
    PUBLIC SystemConfiguration.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/configs
    FILES ${CMAKE_SOURCE_DIR}/system/include/configs/SystemConfiguration.hpp
)

target_link_libraries(system_configs_SystemConfiguration
    PUBLIC
        yaml-cpp::yaml-cpp
)


