# Install a new operation into an existing runtime copy; never run a build here.
file(MAKE_DIRECTORY "${WORK_DIRECTORY}/plugins")
set(new_plugin "${WORK_DIRECTORY}/plugins/new_operation.dll")
file(REMOVE "${new_plugin}")
file(COPY_FILE "${CALC_EXECUTABLE}" "${WORK_DIRECTORY}/calc.exe")
file(SHA256 "${WORK_DIRECTORY}/calc.exe" original_hash)
file(COPY_FILE "${SINE_DLL}" "${WORK_DIRECTORY}/plugins/unrelated_a.dll")
file(COPY_FILE "${POWER_DLL}" "${WORK_DIRECTORY}/plugins/unrelated_b.dll")
file(WRITE "${WORK_DIRECTORY}/input.txt" "2^4 + sin(90)\n")
execute_process(
    COMMAND "${WORK_DIRECTORY}/calc.exe"
    WORKING_DIRECTORY "${WORK_DIRECTORY}/plugins"
    INPUT_FILE "${WORK_DIRECTORY}/input.txt"
    OUTPUT_VARIABLE output ERROR_VARIABLE errors RESULT_VARIABLE status TIMEOUT 5
)
string(REPLACE "\r\n" "\n" output "${output}")
file(SHA256 "${WORK_DIRECTORY}/calc.exe" final_hash)
if(NOT status EQUAL 0 OR NOT output STREQUAL "17\n" OR NOT errors STREQUAL "" OR
   NOT original_hash STREQUAL final_hash)
    message(FATAL_ERROR "Dynamic plugin example failed: ${status}, [${output}], [${errors}]")
endif()

# The operation is unavailable before its DLL is added.
file(WRITE "${WORK_DIRECTORY}/input.txt" "difference(7,2)\n")
execute_process(
    COMMAND "${WORK_DIRECTORY}/calc.exe"
    INPUT_FILE "${WORK_DIRECTORY}/input.txt"
    OUTPUT_VARIABLE output ERROR_VARIABLE errors RESULT_VARIABLE status TIMEOUT 5
)
if(NOT status EQUAL 1 OR NOT output STREQUAL "" OR NOT errors MATCHES "Unknown function")
    message(FATAL_ERROR "Expected an unknown function before plugin installation: ${errors}")
endif()

file(COPY_FILE "${NEW_OPERATION_DLL}" "${new_plugin}")
execute_process(
    COMMAND "${WORK_DIRECTORY}/calc.exe"
    INPUT_FILE "${WORK_DIRECTORY}/input.txt"
    OUTPUT_VARIABLE output ERROR_VARIABLE errors RESULT_VARIABLE status TIMEOUT 5
)
string(REPLACE "\r\n" "\n" output "${output}")
file(SHA256 "${WORK_DIRECTORY}/calc.exe" final_hash)
if(NOT status EQUAL 0 OR NOT output STREQUAL "5\n" OR NOT errors STREQUAL "" OR
   NOT original_hash STREQUAL final_hash)
    message(FATAL_ERROR "New DLL failed without rebuilding the calculator: ${errors}")
endif()
