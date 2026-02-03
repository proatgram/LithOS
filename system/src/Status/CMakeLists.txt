add_library(system_status_Status)
add_library(system::Status ALIAS system_status_Status)

target_sources(system_status_Status
    PUBLIC Status.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/Status
    FILES ${CMAKE_SOURCE_DIR}/system/include/Status/Status.hpp
)

target_link_libraries(system_status_Status
    PUBLIC
        indicators::indicators
)

add_library(system_status_Task)
add_library(system::Task ALIAS system_status_Task)

target_sources(system_status_Task
    PUBLIC Task.cpp
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/Status
    FILES ${CMAKE_SOURCE_DIR}/system/include/Status/Task.hpp
)

target_link_libraries(system_status_Task
    PUBLIC
        system::Status
)

