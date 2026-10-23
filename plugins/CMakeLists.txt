function(add_calc_plugin target source)
    add_library(${target} MODULE ${source})
    target_link_libraries(${target} PRIVATE calc_plugin_api)
    target_compile_definitions(${target} PRIVATE CALC_PLUGIN_BUILD)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins"
    )
endfunction()

add_calc_plugin(plugin_stub stub.cpp)
add_calc_plugin(funcsin sine.cpp)
add_calc_plugin(funcdeg power.cpp)
