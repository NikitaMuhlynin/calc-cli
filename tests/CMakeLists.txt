add_test(NAME calc.startup COMMAND $<TARGET_FILE:calc>)

add_test(NAME calc.build_layout
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DPLUGIN_LIBRARY=$<TARGET_FILE:plugin_stub>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_build_layout.cmake"
)

set_tests_properties(calc.startup calc.build_layout PROPERTIES TIMEOUT 10)

foreach(suite IN ITEMS arithmetic invalid console)
    add_test(NAME calc.${suite}
        COMMAND "${CMAKE_COMMAND}"
            "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
            "-DWORK_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${suite}"
            "-DSUITE=${suite}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/check_calculator.cmake"
    )
    set_tests_properties(calc.${suite} PROPERTIES TIMEOUT 20)
endforeach()

add_executable(plugin_contract_tests plugin_contract_tests.cpp)
target_link_libraries(plugin_contract_tests PRIVATE calc_plugin_api)

add_library(plugin_probe MODULE fixtures/plugin_probe.cpp)
target_link_libraries(plugin_probe PRIVATE calc_plugin_api)
target_compile_definitions(plugin_probe PRIVATE CALC_PLUGIN_BUILD)
set_target_properties(plugin_probe PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests/fixtures/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests/fixtures/$<CONFIG>"
)

add_dependencies(plugin_contract_tests plugin_stub plugin_probe)
add_test(NAME calc.plugin_contract
    COMMAND plugin_contract_tests $<TARGET_FILE:plugin_stub> $<TARGET_FILE:plugin_probe>
)
set_tests_properties(calc.plugin_contract PROPERTIES TIMEOUT 10)

# Compile the public ABI as C, independently of the C++ SDK.
enable_language(C)
add_library(plugin_api_c_check OBJECT plugin_api_c_check.c)
target_link_libraries(plugin_api_c_check PRIVATE calc_plugin_api)
set_target_properties(plugin_api_c_check PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
