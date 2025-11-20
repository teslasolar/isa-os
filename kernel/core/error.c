/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Error Handling Implementation (error.c)
 * Comprehensive error handling and reporting
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/error.h"
#include "../include/isa_types.h"

// ── External Functions ──
extern void kprint(const char *str);
extern void kprinthex(u32 val);
extern u64 system_ticks;
extern void audit_write(isa_level_t level, u32 agent_id, const char *action, const char *data);
extern u32 current_agent;
extern isa_level_t agents[];

// ── Error Statistics ──
static error_stats_t error_stats;
static error_context_t last_error_ctx;
static error_context_t error_history[32];  // Ring buffer of last 32 errors
static u32 error_history_index = 0;

// ── Error String Table ──
static const char *error_strings[] = {
    [ISA_OK] = "Success",
    [ISA_ERR_GENERIC] = "Generic error",
    [ISA_ERR_NOT_IMPLEMENTED] = "Not implemented",
    [ISA_ERR_INVALID_PARAM] = "Invalid parameter",
    [ISA_ERR_NULL_POINTER] = "Null pointer",
    [ISA_ERR_OUT_OF_BOUNDS] = "Out of bounds",
    [ISA_ERR_TIMEOUT] = "Timeout",
    [ISA_ERR_BUSY] = "Resource busy",
    [ISA_ERR_NOT_FOUND] = "Not found",
    [ISA_ERR_ALREADY_EXISTS] = "Already exists",
    [ISA_ERR_NOT_SUPPORTED] = "Not supported",

    [ISA_ERR_NO_MEMORY] = "Out of memory",
    [ISA_ERR_NO_RESOURCE] = "No resource available",
    [ISA_ERR_RESOURCE_BUSY] = "Resource busy",
    [ISA_ERR_RESOURCE_LOCKED] = "Resource locked",
    [ISA_ERR_QUOTA_EXCEEDED] = "Quota exceeded",

    [ISA_ERR_IO] = "I/O error",
    [ISA_ERR_IO_TIMEOUT] = "I/O timeout",
    [ISA_ERR_IO_CRC] = "CRC error",
    [ISA_ERR_IO_OVERFLOW] = "I/O overflow",
    [ISA_ERR_IO_UNDERFLOW] = "I/O underflow",
    [ISA_ERR_DEVICE_NOT_FOUND] = "Device not found",
    [ISA_ERR_DEVICE_FAULT] = "Device fault",
    [ISA_ERR_DEVICE_OFFLINE] = "Device offline",

    [ISA_ERR_PERMISSION_DENIED] = "Permission denied",
    [ISA_ERR_ACCESS_DENIED] = "Access denied",
    [ISA_ERR_AUTH_FAILED] = "Authentication failed",
    [ISA_ERR_SIGNATURE_INVALID] = "Invalid signature",
    [ISA_ERR_CERT_EXPIRED] = "Certificate expired",
    [ISA_ERR_CERT_INVALID] = "Invalid certificate",

    [ISA_ERR_AGENT_NOT_FOUND] = "Agent not found",
    [ISA_ERR_AGENT_TERMINATED] = "Agent terminated",
    [ISA_ERR_AGENT_BLOCKED] = "Agent blocked",
    [ISA_ERR_AGENT_DEADLOCK] = "Agent deadlock detected",
    [ISA_ERR_NO_AGENT_SLOTS] = "No agent slots available",

    [ISA_ERR_FILE_NOT_FOUND] = "File not found",
    [ISA_ERR_FILE_EXISTS] = "File exists",
    [ISA_ERR_FILE_TOO_LARGE] = "File too large",
    [ISA_ERR_DISK_FULL] = "Disk full",
    [ISA_ERR_BAD_INODE] = "Bad inode",
    [ISA_ERR_CORRUPTED_FS] = "Corrupted filesystem",

    [ISA_ERR_NETWORK] = "Network error",
    [ISA_ERR_CONNECTION_REFUSED] = "Connection refused",
    [ISA_ERR_CONNECTION_TIMEOUT] = "Connection timeout",
    [ISA_ERR_CONNECTION_RESET] = "Connection reset",
    [ISA_ERR_HOST_UNREACHABLE] = "Host unreachable",
    [ISA_ERR_PROTOCOL_ERROR] = "Protocol error",

    [ISA_ERR_L0_SENSOR_FAULT] = "L0: Sensor fault",
    [ISA_ERR_L0_ACTUATOR_FAULT] = "L0: Actuator fault",
    [ISA_ERR_L1_CONTROL_FAULT] = "L1: Control fault",
    [ISA_ERR_L1_SETPOINT_INVALID] = "L1: Invalid setpoint",
    [ISA_ERR_L2_ALARM_QUEUE_FULL] = "L2: Alarm queue full",
    [ISA_ERR_L3_BATCH_FAILED] = "L3: Batch failed",
    [ISA_ERR_L3_RECIPE_NOT_FOUND] = "L3: Recipe not found",
    [ISA_ERR_L3_EQUIPMENT_BUSY] = "L3: Equipment busy",
    [ISA_ERR_L4_ERP_SYNC_FAILED] = "L4: ERP sync failed",

    [ISA_ERR_KERNEL_PANIC] = "KERNEL PANIC",
    [ISA_ERR_HARDWARE_FAULT] = "Hardware fault",
    [ISA_ERR_WATCHDOG_TIMEOUT] = "Watchdog timeout",
    [ISA_ERR_STACK_OVERFLOW] = "Stack overflow",
    [ISA_ERR_DOUBLE_FAULT] = "Double fault",
    [ISA_ERR_MEMORY_CORRUPTION] = "Memory corruption",
    [ISA_ERR_SECURITY_BREACH] = "SECURITY BREACH"
};

// ────────────────────────────────────────────────────────────────────────
// Helper Functions
// ────────────────────────────────────────────────────────────────────────

static void *memset(void *dest, int val, u32 count) {
    u8 *d = (u8 *)dest;
    while (count--) *d++ = (u8)val;
    return dest;
}

static void memcpy_internal(void *dest, const void *src, u32 count) {
    u8 *d = (u8 *)dest;
    const u8 *s = (const u8 *)src;
    while (count--) *d++ = *s++;
}

// ────────────────────────────────────────────────────────────────────────
// Error Handling Implementation
// ────────────────────────────────────────────────────────────────────────

void error_init(void) {
    memset(&error_stats, 0, sizeof(error_stats));
    memset(&last_error_ctx, 0, sizeof(last_error_ctx));
    memset(error_history, 0, sizeof(error_history));
    error_history_index = 0;

    kprint("[ERROR] Error handling initialized\n");
}

isa_error_t error_report(isa_error_t code,
                         error_severity_t severity,
                         const char *file,
                         u32 line,
                         const char *function,
                         const char *message) {

    // Update statistics
    error_stats.total_errors++;
    error_stats.errors_by_severity[severity]++;
    error_stats.last_error = code;
    error_stats.last_error_time = system_ticks;

    // Create error context
    error_context_t ctx;
    ctx.code = code;
    ctx.severity = severity;
    ctx.file = file;
    ctx.line = line;
    ctx.function = function;
    ctx.message = message;
    ctx.timestamp = system_ticks;
    ctx.level = ISA_L0_FIELD;  // Default, will be set by caller
    ctx.agent_id = current_agent;

    // Save to last error
    memcpy_internal(&last_error_ctx, &ctx, sizeof(error_context_t));

    // Save to error history (ring buffer)
    memcpy_internal(&error_history[error_history_index], &ctx, sizeof(error_context_t));
    error_history_index = (error_history_index + 1) % 32;

    // Log to console
    const char *severity_str[] = {"INFO", "WARN", "ERROR", "CRIT", "FATAL"};

    kprint("\n[");
    kprint(severity_str[severity]);
    kprint("] ");
    kprint(error_string(code));
    kprint("\n");

    if (message) {
        kprint("  Message: ");
        kprint(message);
        kprint("\n");
    }

    if (file) {
        kprint("  Location: ");
        kprint(file);
        kprint(":");
        // Would need itoa for line number
        kprint("\n");
    }

    if (function) {
        kprint("  Function: ");
        kprint(function);
        kprint("\n");
    }

    // Write to audit log for critical/fatal errors
    if (severity >= ERROR_SEVERITY_CRITICAL) {
        audit_write(ctx.level, ctx.agent_id, "ERROR", error_string(code));
    }

    // Handle fatal errors
    if (severity == ERROR_SEVERITY_FATAL) {
        kernel_panic(message ? message : "Fatal error");
    }

    return code;
}

const char *error_string(isa_error_t code) {
    if (code >= ISA_ERR_MAX) {
        return "Unknown error";
    }

    const char *str = error_strings[code];
    return str ? str : "Unknown error";
}

error_stats_t *error_get_stats(void) {
    return &error_stats;
}

void error_clear_stats(void) {
    memset(&error_stats, 0, sizeof(error_stats));
}

error_context_t *error_get_last(void) {
    return &last_error_ctx;
}

void kernel_panic(const char *message) {
    // Disable interrupts
    __asm__ __volatile__("cli");

    // Print panic message
    kprint("\n\n");
    kprint("════════════════════════════════════════════════════════════════\n");
    kprint("  KERNEL PANIC - System Halted\n");
    kprint("════════════════════════════════════════════════════════════════\n");
    kprint("\n");

    if (message) {
        kprint("Reason: ");
        kprint(message);
        kprint("\n");
    }

    kprint("\nLast error: ");
    kprint(error_string(last_error_ctx.code));
    kprint("\n");

    if (last_error_ctx.file) {
        kprint("File: ");
        kprint(last_error_ctx.file);
        kprint("\n");
    }

    if (last_error_ctx.function) {
        kprint("Function: ");
        kprint(last_error_ctx.function);
        kprint("\n");
    }

    kprint("\nSystem statistics:\n");
    kprint("  Total errors: ");
    kprinthex(error_stats.total_errors);
    kprint("\n  System ticks: ");
    kprinthex((u32)system_ticks);
    kprint("\n");

    kprint("\n");
    kprint("════════════════════════════════════════════════════════════════\n");
    kprint("  System halted. Please reboot.\n");
    kprint("════════════════════════════════════════════════════════════════\n");

    // Write to audit log
    audit_write(ISA_L0_FIELD, current_agent, "KERNEL_PANIC", message);

    // Halt the CPU
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
