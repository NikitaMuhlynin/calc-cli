#ifndef CALC_PLUGIN_API_H
#define CALC_PLUGIN_API_H

#include <stdint.h>

#if defined(_WIN32)
#define CALC_PLUGIN_CALL __cdecl
#if defined(CALC_PLUGIN_BUILD)
#define CALC_PLUGIN_EXPORT __declspec(dllexport)
#else
#define CALC_PLUGIN_EXPORT
#endif
#else
#define CALC_PLUGIN_CALL
#define CALC_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
#define CALC_PLUGIN_NOEXCEPT noexcept
extern "C" {
#else
#define CALC_PLUGIN_NOEXCEPT
#endif

#define CALC_PLUGIN_ABI_VERSION 1u
#define CALC_PLUGIN_ENTRY_NAME "calc_plugin_get_api"
#define CALC_PLUGIN_NAME_CAPACITY 64u
#define CALC_PLUGIN_MAX_ARGUMENTS 16u

typedef uint32_t CalcPluginStatus;
#define CALC_PLUGIN_OK 0u
#define CALC_PLUGIN_INVALID_ARGUMENT 1u
#define CALC_PLUGIN_DOMAIN_ERROR 2u
#define CALC_PLUGIN_NUMERIC_ERROR 3u
#define CALC_PLUGIN_INTERNAL_ERROR 4u
#define CALC_PLUGIN_INCOMPATIBLE_ABI 5u

#define CALC_PLUGIN_FUNCTION 1u
#define CALC_PLUGIN_INFIX 2u
#define CALC_PLUGIN_ASSOC_NONE 0u
#define CALC_PLUGIN_ASSOC_LEFT 1u
#define CALC_PLUGIN_ASSOC_RIGHT 2u

typedef CalcPluginStatus(CALC_PLUGIN_CALL* CalcPluginInvoke)(
    const double* arguments, uint32_t argument_count, double* result,
    char* error_message, uint32_t error_capacity) CALC_PLUGIN_NOEXCEPT;

#pragma pack(push, 8)
typedef struct CalcPluginApi
{
    uint32_t abi_version;
    uint32_t struct_size;
    char name[CALC_PLUGIN_NAME_CAPACITY];
    uint32_t kind;
    uint32_t argument_count;
    uint32_t precedence;
    uint32_t associativity;
    CalcPluginInvoke invoke;
} CalcPluginApi;
#pragma pack(pop)

typedef CalcPluginStatus(CALC_PLUGIN_CALL* CalcPluginGetApi)(
    uint32_t requested_version, uint32_t output_size,
    CalcPluginApi* output) CALC_PLUGIN_NOEXCEPT;

CALC_PLUGIN_EXPORT CalcPluginStatus CALC_PLUGIN_CALL calc_plugin_get_api(
    uint32_t requested_version, uint32_t output_size,
    CalcPluginApi* output) CALC_PLUGIN_NOEXCEPT;

#ifdef __cplusplus
}
#endif

#endif
