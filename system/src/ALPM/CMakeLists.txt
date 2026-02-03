add_library(system_ALPM)
add_library(system::ALPM ALIAS system_ALPM)

target_sources(system_ALPM
    PUBLIC ALPM.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/ALPM.hpp
)

target_link_libraries(system_ALPM
    PUBLIC
        PkgConfig::libalpm
        system::ALPM::Transaction
        system::ALPM::Config
        system::ALPM::Database
)

add_library(system_ALPM_Database)
add_library(system::ALPM::Database ALIAS system_ALPM_Database)

target_sources(system_ALPM_Database
    PUBLIC Database.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/Database.hpp
)

target_link_libraries(system_ALPM_Database
    PUBLIC
        PkgConfig::libalpm
        system::Utils
)

add_library(system_ALPM_Transaction)
add_library(system::ALPM::Transaction ALIAS system_ALPM_Transaction)

target_sources(system_ALPM_Transaction
    PUBLIC Transaction.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/Transaction.hpp
)

target_link_libraries(system_ALPM_Transaction
    PUBLIC
        PkgConfig::libalpm
        system::ALPM::Package
        system::ALPM::Database
        system::Utils
)

add_library(system_ALPM_Package)
add_library(system::ALPM::Package ALIAS system_ALPM_Package)

target_sources(system_ALPM_Package
    PUBLIC Package.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/Package.hpp
)

target_link_libraries(system_ALPM_Package
    PUBLIC
        PkgConfig::libalpm
        system::ALPM
        system::ALPM::Dependency
        system::ALPM::Database
        system::ALPM::File
)

add_library(system_ALPM_Dependency)
add_library(system::ALPM::Dependency ALIAS system_ALPM_Dependency)

target_sources(system_ALPM_Dependency
    PUBLIC Dependency.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/Dependency.hpp
)

target_link_libraries(system_ALPM_Dependency
    PUBLIC
        PkgConfig::libalpm
        system::ALPM::Package
)

add_library(system_ALPM_File)
add_library(system::ALPM::File ALIAS system_ALPM_File)

target_sources(system_ALPM_File
    PUBLIC File.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/File.hpp
)

target_link_libraries(system_ALPM_File
    PUBLIC
        PkgConfig::libalpm
        system::ALPM::Package
)

add_library(system_ALPM_Config)
add_library(system::ALPM::Config ALIAS system_ALPM_Config)

target_sources(system_ALPM_Config
    PUBLIC Config.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/Config.hpp
)

add_library(system_ALPM_Signature)
add_library(system::ALPM::Signature ALIAS system_ALPM_Signature)

target_sources(system_ALPM_Signature
    PUBLIC Signature.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/ALPM
    FILES ${CMAKE_SOURCE_DIR}/system/include/ALPM/Signature.hpp
)

target_link_libraries(system_ALPM_Signature
    PUBLIC
        PkgConfig::libalpm
        system::ALPM
        system::ALPM::Package
        system::ALPM::Database
)
