include(FetchContent)
set(gtest_force_shared_crt ON CACHE BOOL "Use the same MSVC runtime as calc-cli" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "GoogleMock is not needed" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Do not install test dependencies" FORCE)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG 063de7e9578f82b369302001269680b4b1553359 # v1.18.0
)
FetchContent_MakeAvailable(googletest)

add_executable(calc_tests calculator_tests.cpp plugin_tests.cpp
    plugin_contract_tests.cpp plugin_lifetime_tests.cpp)
target_link_libraries(calc_tests PRIVATE calc_core GTest::gtest_main)
target_compile_definitions(calc_tests PRIVATE
    CALC_TEST_FIXTURE_DIRECTORY="$<TARGET_FILE_DIR:plugin_probe>"
    CALC_TEST_WORK_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources"
)
add_dependencies(calc_tests plugin_stub funcsin funcdeg plugin_probe
    plugin_no_entry plugin_bad_metadata plugin_refused)
include(GoogleTest)
gtest_discover_tests(calc_tests TEST_PREFIX unit. DISCOVERY_MODE PRE_TEST
    PROPERTIES TIMEOUT 10 LABELS unit)

add_test(NAME calc.startup
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DWORK_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/startup"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_startup.cmake"
)

add_test(NAME calc.build_layout
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DPLUGIN_LIBRARY=$<TARGET_FILE:plugin_stub>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_build_layout.cmake"
)

set_tests_properties(calc.startup calc.build_layout PROPERTIES TIMEOUT 10)

add_library(plugin_probe MODULE fixtures/plugin_probe.cpp)
target_link_libraries(plugin_probe PRIVATE calc_plugin_api)
target_compile_definitions(plugin_probe PRIVATE CALC_PLUGIN_BUILD)
set_target_properties(plugin_probe PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests/fixtures/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests/fixtures/$<CONFIG>"
)

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

add_test(NAME calc.plugins_without_rebuild
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DSINE_DLL=$<TARGET_FILE:funcsin>"
        "-DPOWER_DLL=$<TARGET_FILE:funcdeg>"
        "-DNEW_OPERATION_DLL=$<TARGET_FILE:plugin_pair>"
        "-DWORK_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/without_rebuild"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_plugins_without_rebuild.cmake"
)
set_tests_properties(calc.plugins_without_rebuild PROPERTIES TIMEOUT 10)
set_tests_properties(calc.startup calc.build_layout calc.plugin_loading calc.plugins_without_rebuild
    PROPERTIES LABELS integration)
