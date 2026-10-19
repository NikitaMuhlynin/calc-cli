add_test(NAME calc.startup COMMAND $<TARGET_FILE:calc>)

add_test(NAME calc.build_layout
    COMMAND "${CMAKE_COMMAND}"
        "-DCALC_EXECUTABLE=$<TARGET_FILE:calc>"
        "-DPLUGIN_LIBRARY=$<TARGET_FILE:plugin_stub>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_build_layout.cmake"
)

set_tests_properties(calc.startup calc.build_layout PROPERTIES TIMEOUT 10)
