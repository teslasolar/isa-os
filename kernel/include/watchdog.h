/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Watchdog Timer System (watchdog.h)
 * Per-agent watchdog for fault detection and recovery
 * ════════════════════════════════════════════════════════════════════════
 */

#ifndef ISA_WATCHDOG_H
#define ISA_WATCHDOG_H

#include "isa_types.h"
#include "error.h"

// ── Watchdog Configuration ──
#define WATCHDOG_MAX 64              // Max watchdogs
#define WATCHDOG_DEFAULT_TIMEOUT 5000 // 5 seconds default

// ── Watchdog States ──
typedef enum {
    WATCHDOG_DISABLED,    // Watchdog not active
    WATCHDOG_ENABLED,     // Active, monitoring
    WATCHDOG_EXPIRED,     // Timeout occurred
    WATCHDOG_TRIGGERED    // Recovery action taken
} watchdog_state_t;

// ── Recovery Actions ──
typedef enum {
    RECOVERY_NONE,           // No action
    RECOVERY_LOG,            // Log only
    RECOVERY_ALARM,          // Raise alarm
    RECOVERY_RESTART_AGENT,  // Restart agent
    RECOVERY_SAFE_STATE,     // Move process to safe state
    RECOVERY_SYSTEM_REBOOT   // Reboot entire system
} recovery_action_t;

// ── Watchdog Descriptor ──
typedef struct {
    u32 id;                      // Watchdog ID
    u32 agent_id;                // Monitored agent ID
    isa_level_t level;           // ISA level
    u64 timeout_ms;              // Timeout in milliseconds
    u64 last_kick;               // Last heartbeat timestamp
    watchdog_state_t state;      // Current state
    recovery_action_t recovery;  // Recovery action
    void (*recovery_fn)(u32);    // Custom recovery function
    u32 expiry_count;            // Times watchdog expired
    char name[32];               // Watchdog name
} watchdog_t;

// ── Watchdog Statistics ──
typedef struct {
    u32 total_watchdogs;
    u32 active_watchdogs;
    u32 total_expirations;
    u32 total_recoveries;
} watchdog_stats_t;

// ── Function Declarations ──

/**
 * Initialize watchdog subsystem
 */
void watchdog_init(void);

/**
 * Create new watchdog for agent
 */
u32 watchdog_create(u32 agent_id,
                    u64 timeout_ms,
                    recovery_action_t recovery,
                    const char *name);

/**
 * Enable watchdog
 */
isa_error_t watchdog_enable(u32 watchdog_id);

/**
 * Disable watchdog
 */
isa_error_t watchdog_disable(u32 watchdog_id);

/**
 * Kick watchdog (reset timer)
 */
isa_error_t watchdog_kick(u32 watchdog_id);

/**
 * Check all watchdogs (called from timer interrupt)
 */
void watchdog_check_all(void);

/**
 * Get watchdog info
 */
watchdog_t *watchdog_get(u32 watchdog_id);

/**
 * Get watchdog statistics
 */
watchdog_stats_t *watchdog_get_stats(void);

/**
 * List all watchdogs
 */
void watchdog_list(void);

/**
 * Delete watchdog
 */
isa_error_t watchdog_delete(u32 watchdog_id);

#endif // ISA_WATCHDOG_H
