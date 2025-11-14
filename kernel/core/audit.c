/**
 * ════════════════════════════════════════════════════════════════════════
 * Audit Logging System (audit.c)
 * 21 CFR Part 11 / EU Annex 11 Compliance
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"

// ── Audit Log Storage ──
static audit_log_t audit_logs[MAX_AUDIT_LOGS];
static u32 audit_log_count = 0;
static u32 audit_log_head = 0;

// ── CRC64 for Integrity ──
static u64 crc64_table[256];
static bool crc64_initialized = false;

static void crc64_init(void) {
    if (crc64_initialized) return;

    const u64 polynomial = 0x42F0E1EBA9EA3693ULL;

    for (u32 i = 0; i < 256; i++) {
        u64 crc = i;
        for (u32 j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
        crc64_table[i] = crc;
    }

    crc64_initialized = true;
}

static u64 crc64(const void *data, u32 length) {
    const u8 *bytes = (const u8 *)data;
    u64 crc = 0xFFFFFFFFFFFFFFFFULL;

    for (u32 i = 0; i < length; i++) {
        u8 index = (u8)(crc ^ bytes[i]);
        crc = (crc >> 8) ^ crc64_table[index];
    }

    return crc ^ 0xFFFFFFFFFFFFFFFFULL;
}

// ────────────────────────────────────────────────────────────────────────
// Audit Functions
// ────────────────────────────────────────────────────────────────────────

/**
 * Initialize audit subsystem
 */
void audit_init(void) {
    crc64_init();

    for (u32 i = 0; i < MAX_AUDIT_LOGS; i++) {
        audit_logs[i].timestamp = 0;
        audit_logs[i].action[0] = 0;
        audit_logs[i].data[0] = 0;
    }
}

/**
 * Write audit log entry
 * WHO: Which agent/user performed action
 * WHAT: What action was performed
 * WHEN: Timestamp (automatic)
 * WHERE: Which ISA level
 * WHY: Context/reason
 */
void audit_write(isa_level_t level, u32 agent_id, const char *action, const char *data) {
    if (audit_log_count >= MAX_AUDIT_LOGS) {
        // Circular buffer: overwrite oldest
        audit_log_head = (audit_log_head + 1) % MAX_AUDIT_LOGS;
    } else {
        audit_log_count++;
    }

    u32 index = (audit_log_head + audit_log_count - 1) % MAX_AUDIT_LOGS;
    audit_log_t *log = &audit_logs[index];

    // Capture timestamp
    extern u64 system_ticks;
    log->timestamp = system_ticks;

    // Record details
    log->level = level;
    log->agent_id = agent_id;
    log->user_id = 0; // In full system, would track authenticated user

    // Copy action string
    u32 i;
    for (i = 0; i < AUDIT_ACTION_MAX - 1 && action[i]; i++) {
        log->action[i] = action[i];
    }
    log->action[i] = 0;

    // Copy data string
    for (i = 0; i < AUDIT_DATA_MAX - 1 && data && data[i]; i++) {
        log->data[i] = data[i];
    }
    log->data[i] = 0;

    // Calculate integrity checksum (excluding CRC field itself)
    log->crc = crc64(log, sizeof(audit_log_t) - sizeof(u64));
}

/**
 * Verify audit log integrity
 */
bool audit_verify(u32 index) {
    if (index >= audit_log_count) {
        return false;
    }

    audit_log_t *log = &audit_logs[index];
    u64 stored_crc = log->crc;

    // Recalculate CRC
    u64 calculated_crc = crc64(log, sizeof(audit_log_t) - sizeof(u64));

    return (stored_crc == calculated_crc);
}

/**
 * Retrieve audit log
 */
audit_log_t *audit_get(u32 index) {
    if (index >= audit_log_count) {
        return 0;
    }

    return &audit_logs[index];
}

/**
 * Search audit logs by criteria
 */
u32 audit_search(isa_level_t level, u32 agent_id, audit_log_t *results, u32 max_results) {
    u32 count = 0;

    for (u32 i = 0; i < audit_log_count && count < max_results; i++) {
        audit_log_t *log = &audit_logs[i];

        bool match = true;

        // Filter by level (0xFF = all levels)
        if (level != 0xFF && log->level != level) {
            match = false;
        }

        // Filter by agent (0xFFFFFFFF = all agents)
        if (agent_id != 0xFFFFFFFF && log->agent_id != agent_id) {
            match = false;
        }

        if (match) {
            results[count++] = *log;
        }
    }

    return count;
}

/**
 * Export audit trail (for regulatory compliance)
 */
u32 audit_export(u8 *buffer, u32 buffer_size) {
    // Export audit logs in standardized format (e.g., CSV, JSON)
    // For regulatory audits and inspections

    u32 offset = 0;
    const char *header = "Timestamp,Level,Agent,Action,Data,CRC\n";

    // Write header
    for (u32 i = 0; header[i] && offset < buffer_size; i++) {
        buffer[offset++] = header[i];
    }

    // Write each log entry
    for (u32 i = 0; i < audit_log_count && offset < buffer_size - 256; i++) {
        audit_log_t *log = &audit_logs[i];

        // Format: timestamp,level,agent,action,data,crc
        // (Simplified - real implementation would use proper CSV escaping)

        // Would use sprintf here, but keeping minimal libc
        // For now, just calculate required size

        offset += 128; // Placeholder
    }

    return offset;
}
