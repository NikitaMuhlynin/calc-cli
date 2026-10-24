add_test(NAME calc.startup COMMAND $<TARGET_FILE:calc>)

add_test(NAME calc.build_layout
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DPLUGIN_LIBRARY=$<TARGET_FILE:plugin_stub>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_build_layout.cmake"
)

set_tests_properties(calc.startup calc.build_layout PROPERTIES TIMEOUT 10)

foreach(suite IN ITEMS arithmetic invalid console math)
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

foreach(fixture IN ITEMS difference pair no_entry bad_metadata unreliable refused)
    add_library(plugin_${fixture} MODULE fixtures/plugin_${fixture}.cpp)
    target_link_libraries(plugin_${fixture} PRIVATE calc_plugin_api)
    target_compile_definitions(plugin_${fixture} PRIVATE CALC_PLUGIN_BUILD)
    set_target_properties(plugin_${fixture} PROPERTIES
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests/fixtures/$<CONFIG>"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests/fixtures/$<CONFIG>"
    )
endforeach()

add_test(NAME calc.plugin_loading
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DIDENTITY_DLL=$<TARGET_FILE:plugin_stub>"
        "-DPROBE_DLL=$<TARGET_FILE:plugin_probe>"
        "-DDIFFERENCE_DLL=$<TARGET_FILE:plugin_difference>"
        "-DPAIR_DLL=$<TARGET_FILE:plugin_pair>"
        "-DNO_ENTRY_DLL=$<TARGET_FILE:plugin_no_entry>"
        "-DBAD_METADATA_DLL=$<TARGET_FILE:plugin_bad_metadata>"
        "-DUNRELIABLE_DLL=$<TARGET_FILE:plugin_unreliable>"
        "-DREFUSED_DLL=$<TARGET_FILE:plugin_refused>"
        "-DWORK_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/loading"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_plugin_loading.cmake"
)
set_tests_properties(calc.plugin_loading PROPERTIES TIMEOUT 30)

add_executable(plugin_lifetime_tests plugin_lifetime_tests.cpp)
target_link_libraries(plugin_lifetime_tests PRIVATE calc_core)
add_dependencies(plugin_lifetime_tests plugin_stub plugin_no_entry plugin_bad_metadata plugin_refused)
add_test(NAME calc.plugin_lifetime
    COMMAND plugin_lifetime_tests $<TARGET_FILE:plugin_stub>
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources"
        $<TARGET_FILE:plugin_no_entry> $<TARGET_FILE:plugin_bad_metadata> $<TARGET_FILE:plugin_refused>
)
set_tests_properties(calc.plugin_lifetime PROPERTIES TIMEOUT 10)

add_executable(math_plugins_tests math_plugins_tests.cpp)
target_link_libraries(math_plugins_tests PRIVATE calc_core)
add_dependencies(math_plugins_tests funcsin funcdeg)
add_test(NAME calc.math_precision COMMAND math_plugins_tests $<TARGET_FILE_DIR:funcsin>)
set_tests_properties(calc.math_precision PROPERTIES TIMEOUT 10)

add_test(NAME calc.plugins_without_rebuild
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DSINE_DLL=$<TARGET_FILE:funcsin>"
        "-DPOWER_DLL=$<TARGET_FILE:funcdeg>"
        "-DWORK_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/without_rebuild"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_plugins_without_rebuild.cmake"
)
set_tests_properties(calc.plugins_without_rebuild PROPERTIES TIMEOUT 10)
