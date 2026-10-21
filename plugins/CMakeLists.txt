# SDK example: the operation name is identity, independent of the DLL filename.
add_library(plugin_stub MODULE stub.cpp)
target_link_libraries(plugin_stub PRIVATE calc_plugin_api)
target_compile_definitions(plugin_stub PRIVATE CALC_PLUGIN_BUILD)
set_target_properties(plugin_stub PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins"
)
