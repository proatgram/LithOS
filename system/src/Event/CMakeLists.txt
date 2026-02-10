add_library(system_event INTERFACE)
add_library(system::Event ALIAS system_event)

target_sources(system_event
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_SOURCE_DIR}/system/include/Event
        FILES ${CMAKE_SOURCE_DIR}/system/include/Event.hpp
)
