# Temporary module used to verify the DLL build and output layout.
add_library(plugin_stub MODULE stub.cpp)
set_target_properties(plugin_stub PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins"
)
