/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Error Handling Framework (error.h)
 * Standardized error codes and handling for all kernel subsystems
 * ════════════════════════════════════════════════════════════════════════
 */

#ifndef ISA_ERROR_H
#define ISA_ERROR_H

#include "isa_types.h"

// ── Error Codes ──
typedef enum {
    // Success
    ISA_OK = 0,

    // General errors (1-99)
    ISA_ERR_GENERIC = 1,
    ISA_ERR_NOT_IMPLEMENTED = 2,
    ISA_ERR_INVALID_PARAM = 3,
    ISA_ERR_NULL_POINTER = 4,
    ISA_ERR_OUT_OF_BOUNDS = 5,
    ISA_ERR_TIMEOUT = 6,
    ISA_ERR_BUSY = 7,
    ISA_ERR_NOT_FOUND = 8,
    ISA_ERR_ALREADY_EXISTS = 9,
    ISA_ERR_NOT_SUPPORTED = 10,

    // Resource errors (100-199)
    ISA_ERR_NO_MEMORY = 100,
    ISA_ERR_NO_RESOURCE = 101,
    ISA_ERR_RESOURCE_BUSY = 102,
    ISA_ERR_RESOURCE_LOCKED = 103,
    ISA_ERR_QUOTA_EXCEEDED = 104,

    // I/O errors (200-299)
    ISA_ERR_IO = 200,
    ISA_ERR_IO_TIMEOUT = 201,
    ISA_ERR_IO_CRC = 202,
    ISA_ERR_IO_OVERFLOW = 203,
    ISA_ERR_IO_UNDERFLOW = 204,
    ISA_ERR_DEVICE_NOT_FOUND = 205,
    ISA_ERR_DEVICE_FAULT = 206,
    ISA_ERR_DEVICE_OFFLINE = 207,

    // Permission/Security errors (300-399)
    ISA_ERR_PERMISSION_DENIED = 300,
    ISA_ERR_ACCESS_DENIED = 301,
    ISA_ERR_AUTH_FAILED = 302,
    ISA_ERR_SIGNATURE_INVALID = 303,
    ISA_ERR_CERT_EXPIRED = 304,
    ISA_ERR_CERT_INVALID = 305,

    // Agent/Process errors (400-499)
    ISA_ERR_AGENT_NOT_FOUND = 400,
    ISA_ERR_AGENT_TERMINATED = 401,
    ISA_ERR_AGENT_BLOCKED = 402,
    ISA_ERR_AGENT_DEADLOCK = 403,
    ISA_ERR_NO_AGENT_SLOTS = 404,

    // Filesystem errors (500-599)
    ISA_ERR_FILE_NOT_FOUND = 500,
    ISA_ERR_FILE_EXISTS = 501,
    ISA_ERR_FILE_TOO_LARGE = 502,
    ISA_ERR_DISK_FULL = 503,
    ISA_ERR_BAD_INODE = 504,
    ISA_ERR_CORRUPTED_FS = 505,

    // Network errors (600-699)
    ISA_ERR_NETWORK = 600,
    ISA_ERR_CONNECTION_REFUSED = 601,
    ISA_ERR_CONNECTION_TIMEOUT = 602,
    ISA_ERR_CONNECTION_RESET = 603,
    ISA_ERR_HOST_UNREACHABLE = 604,
    ISA_ERR_PROTOCOL_ERROR = 605,

    // ISA-95 Level-specific errors (700-799)
    ISA_ERR_L0_SENSOR_FAULT = 700,
    ISA_ERR_L0_ACTUATOR_FAULT = 701,
    ISA_ERR_L1_CONTROL_FAULT = 710,
    ISA_ERR_L1_SETPOINT_INVALID = 711,
    ISA_ERR_L2_ALARM_QUEUE_FULL = 720,
    ISA_ERR_L3_BATCH_FAILED = 730,
    ISA_ERR_L3_RECIPE_NOT_FOUND = 731,
    ISA_ERR_L3_EQUIPMENT_BUSY = 732,
    ISA_ERR_L4_ERP_SYNC_FAILED = 740,

    // Critical/Fatal errors (900-999)
    ISA_ERR_KERNEL_PANIC = 900,
    ISA_ERR_HARDWARE_FAULT = 901,
    ISA_ERR_WATCHDOG_TIMEOUT = 902,
    ISA_ERR_STACK_OVERFLOW = 903,
    ISA_ERR_DOUBLE_FAULT = 904,
    ISA_ERR_MEMORY_CORRUPTION = 905,
    ISA_ERR_SECURITY_BREACH = 906,

    ISA_ERR_MAX = 1000
} isa_error_t;

// ── Error Severity Levels ──
typedef enum {
    ERROR_SEVERITY_INFO,       // Informational, no action needed
    ERROR_SEVERITY_WARNING,    // Warning, system continues
    ERROR_SEVERITY_ERROR,      // Error, operation failed but system stable
    ERROR_SEVERITY_CRITICAL,   // Critical, system degraded
    ERROR_SEVERITY_FATAL       // Fatal, system must halt
} error_severity_t;

// ── Error Context (for debugging) ──
typedef struct {
    isa_error_t code;          // Error code
    error_severity_t severity; // Severity level
    const char *file;          // Source file
    u32 line;                  // Line number
    const char *function;      // Function name
    const char *message;       // Human-readable message
    u64 timestamp;             // When error occurred
    isa_level_t level;         // Which ISA level
    u32 agent_id;              // Which agent (if applicable)
} error_context_t;

// ── Error Statistics ──
typedef struct {
    u32 total_errors;
    u32 errors_by_severity[5];  // Indexed by error_severity_t
    u32 errors_by_level[5];     // Indexed by isa_level_t
    isa_error_t last_error;
    u64 last_error_time;
} error_stats_t;

// ── Macros for Error Reporting ──
#define ISA_ERROR(code, msg) \
    error_report(code, ERROR_SEVERITY_ERROR, __FILE__, __LINE__, __func__, msg)

#define ISA_WARNING(code, msg) \
    error_report(code, ERROR_SEVERITY_WARNING, __FILE__, __LINE__, __func__, msg)

#define ISA_CRITICAL(code, msg) \
    error_report(code, ERROR_SEVERITY_CRITICAL, __FILE__, __LINE__, __func__, msg)

#define ISA_FATAL(code, msg) \
    error_report(code, ERROR_SEVERITY_FATAL, __FILE__, __LINE__, __func__, msg)

#define ISA_CHECK(expr, code, msg) \
    do { \
        if (!(expr)) { \
            return ISA_ERROR(code, msg); \
        } \
    } while (0)

#define ISA_ASSERT(expr, msg) \
    do { \
        if (!(expr)) { \
            ISA_FATAL(ISA_ERR_KERNEL_PANIC, msg); \
            kernel_panic(msg); \
        } \
    } while (0)

// ── Function Declarations ──

/**
 * Initialize error handling subsystem
 */
void error_init(void);

/**
 * Report an error
 */
isa_error_t error_report(isa_error_t code,
                         error_severity_t severity,
                         const char *file,
                         u32 line,
                         const char *function,
                         const char *message);

/**
 * Get error description string
 */
const char *error_string(isa_error_t code);

/**
 * Get error statistics
 */
error_stats_t *error_get_stats(void);

/**
 * Clear error statistics
 */
void error_clear_stats(void);

/**
 * Get last error context
 */
error_context_t *error_get_last(void);

/**
 * Kernel panic (fatal error, system halts)
 */
void kernel_panic(const char *message) __attribute__((noreturn));

#endif // ISA_ERROR_H
