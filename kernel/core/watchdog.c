/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Watchdog Timer Implementation (watchdog.c)
 * Fault detection and automatic recovery
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/watchdog.h"
#include "../include/isa_types.h"
#include "../include/error.h"

// ── External Functions ──
extern void kprint(const char *str);
extern void kprinthex(u32 val);
extern u64 system_ticks;
extern void audit_write(isa_level_t level, u32 agent_id, const char *action, const char *data);
extern isa_error_t agent_terminate(u32 agent_id);
extern isa_error_t agent_suspend(u32 agent_id);
extern u32 agent_create(isa_level_t level, u32 priority, void (*entry_point)(void), const char *name);
extern void sys_l2_alarm(u32 priority, const char *msg, u32 unused1, u32 unused2);

// ── Watchdog State ──
static watchdog_t watchdogs[WATCHDOG_MAX];
static u32 watchdog_count = 0;
static watchdog_stats_t watchdog_stats;

// ────────────────────────────────────────────────────────────────────────
// Helper Functions
// ────────────────────────────────────────────────────────────────────────

static void *memset(void *dest, int val, u32 count) {
    u8 *d = (u8 *)dest;
    while (count--) *d++ = (u8)val;
    return dest;
}

static void strcpy_internal(char *dest, const char *src) {
    while ((*dest++ = *src++));
}

static u32 strlen_internal(const char *str) {
    u32 len = 0;
    while (str[len]) len++;
    return len;
}

// ────────────────────────────────────────────────────────────────────────
// Recovery Actions
// ────────────────────────────────────────────────────────────────────────

static void recovery_log(watchdog_t *wd) {
    kprint("[WATCHDOG] Timeout: ");
    kprint(wd->name);
    kprint(" (agent ");
    kprinthex(wd->agent_id);
    kprint(")\n");

    audit_write(wd->level, wd->agent_id, "WATCHDOG_TIMEOUT", wd->name);
}

static void recovery_alarm(watchdog_t *wd) {
    recovery_log(wd);

    char msg[128];
    strcpy_internal(msg, "WATCHDOG TIMEOUT: ");

    // Append agent ID (simple itoa)
    u32 pos = strlen_internal(msg);
    u32 agent_id = wd->agent_id;
    u32 digits = 0;
    u32 temp = agent_id;
    do {
        digits++;
        temp /= 10;
    } while (temp > 0);

    for (u32 i = 0; i < digits; i++) {
        msg[pos + digits - i - 1] = '0' + (agent_id % 10);
        agent_id /= 10;
    }
    msg[pos + digits] = 0;

    sys_l2_alarm(1, msg, 0, 0);  // CRITICAL alarm
}

static void recovery_restart_agent(watchdog_t *wd) {
    recovery_alarm(wd);

    kprint("[WATCHDOG] Restarting agent ");
    kprinthex(wd->agent_id);
    kprint("\n");

    // Terminate old agent
    agent_terminate(wd->agent_id);

    // TODO: Restart agent (need entry point stored)
    // For now, just log
    audit_write(wd->level, wd->agent_id, "AGENT_RESTART_REQUESTED", wd->name);
}

static void recovery_safe_state(watchdog_t *wd) {
    recovery_alarm(wd);

    kprint("[WATCHDOG] Moving to safe state (agent ");
    kprinthex(wd->agent_id);
    kprint(")\n");

    // TODO: Implement safe state logic
    // For L0/L1: Stop all actuators
    // For L2: Pause automation
    // For L3/L4: Graceful degradation

    audit_write(wd->level, wd->agent_id, "SAFE_STATE_ENTERED", wd->name);
}

static void recovery_system_reboot(watchdog_t *wd) {
    recovery_alarm(wd);

    kprint("\n[WATCHDOG] CRITICAL: System reboot requested by watchdog\n");
    kprint("Watchdog: ");
    kprint(wd->name);
    kprint("\nAgent: ");
    kprinthex(wd->agent_id);
    kprint("\n");

    audit_write(wd->level, wd->agent_id, "SYSTEM_REBOOT_REQUESTED", wd->name);

    // TODO: Implement graceful reboot
    // For now, kernel panic
    ISA_FATAL(ISA_ERR_WATCHDOG_TIMEOUT, "Watchdog-triggered reboot");
}

// ────────────────────────────────────────────────────────────────────────
// Watchdog Management
// ────────────────────────────────────────────────────────────────────────

void watchdog_init(void) {
    memset(watchdogs, 0, sizeof(watchdogs));
    memset(&watchdog_stats, 0, sizeof(watchdog_stats));
    watchdog_count = 0;

    kprint("[WATCHDOG] Watchdog subsystem initialized\n");
}

u32 watchdog_create(u32 agent_id,
                    u64 timeout_ms,
                    recovery_action_t recovery,
                    const char *name) {
    if (watchdog_count >= WATCHDOG_MAX) {
        return 0xFFFFFFFF;  // No slots
    }

    watchdog_t *wd = &watchdogs[watchdog_count];

    wd->id = watchdog_count;
    wd->agent_id = agent_id;
    wd->level = ISA_L0_FIELD;  // TODO: Get from agent
    wd->timeout_ms = timeout_ms ? timeout_ms : WATCHDOG_DEFAULT_TIMEOUT;
    wd->last_kick = system_ticks;
    wd->state = WATCHDOG_DISABLED;
    wd->recovery = recovery;
    wd->recovery_fn = NULL;
    wd->expiry_count = 0;

    if (name) {
        u32 len = strlen_internal(name);
        if (len >= 32) len = 31;
        for (u32 i = 0; i < len; i++) {
            wd->name[i] = name[i];
        }
        wd->name[len] = 0;
    } else {
        wd->name[0] = 'W';
        wd->name[1] = 'D';
        wd->name[2] = '0' + (watchdog_count % 10);
        wd->name[3] = 0;
    }

    watchdog_stats.total_watchdogs++;

    kprint("[WATCHDOG] Created watchdog ");
    kprinthex(wd->id);
    kprint(": ");
    kprint(wd->name);
    kprint(" (agent ");
    kprinthex(agent_id);
    kprint(", timeout ");
    kprinthex((u32)timeout_ms);
    kprint("ms)\n");

    watchdog_count++;
    return wd->id;
}

isa_error_t watchdog_enable(u32 watchdog_id) {
    if (watchdog_id >= watchdog_count) {
        return ISA_ERR_NOT_FOUND;
    }

    watchdog_t *wd = &watchdogs[watchdog_id];
    wd->state = WATCHDOG_ENABLED;
    wd->last_kick = system_ticks;

    watchdog_stats.active_watchdogs++;

    return ISA_OK;
}

isa_error_t watchdog_disable(u32 watchdog_id) {
    if (watchdog_id >= watchdog_count) {
        return ISA_ERR_NOT_FOUND;
    }

    watchdog_t *wd = &watchdogs[watchdog_id];
    if (wd->state == WATCHDOG_ENABLED) {
        wd->state = WATCHDOG_DISABLED;
        watchdog_stats.active_watchdogs--;
    }

    return ISA_OK;
}

isa_error_t watchdog_kick(u32 watchdog_id) {
    if (watchdog_id >= watchdog_count) {
        return ISA_ERR_NOT_FOUND;
    }

    watchdog_t *wd = &watchdogs[watchdog_id];
    if (wd->state == WATCHDOG_ENABLED || wd->state == WATCHDOG_EXPIRED) {
        wd->last_kick = system_ticks;
        wd->state = WATCHDOG_ENABLED;
    }

    return ISA_OK;
}

void watchdog_check_all(void) {
    for (u32 i = 0; i < watchdog_count; i++) {
        watchdog_t *wd = &watchdogs[i];

        if (wd->state != WATCHDOG_ENABLED) {
            continue;
        }

        // Check if timeout expired
        u64 elapsed_ms = system_ticks - wd->last_kick;
        if (elapsed_ms >= wd->timeout_ms) {
            // Watchdog expired!
            wd->state = WATCHDOG_EXPIRED;
            wd->expiry_count++;
            watchdog_stats.total_expirations++;

            // Perform recovery action
            switch (wd->recovery) {
                case RECOVERY_LOG:
                    recovery_log(wd);
                    break;

                case RECOVERY_ALARM:
                    recovery_alarm(wd);
                    break;

                case RECOVERY_RESTART_AGENT:
                    recovery_restart_agent(wd);
                    break;

                case RECOVERY_SAFE_STATE:
                    recovery_safe_state(wd);
                    break;

                case RECOVERY_SYSTEM_REBOOT:
                    recovery_system_reboot(wd);
                    break;

                case RECOVERY_NONE:
                default:
                    break;
            }

            // Call custom recovery function if set
            if (wd->recovery_fn) {
                wd->recovery_fn(wd->agent_id);
            }

            wd->state = WATCHDOG_TRIGGERED;
            watchdog_stats.total_recoveries++;
        }
    }
}

watchdog_t *watchdog_get(u32 watchdog_id) {
    if (watchdog_id >= watchdog_count) {
        return NULL;
    }
    return &watchdogs[watchdog_id];
}

watchdog_stats_t *watchdog_get_stats(void) {
    return &watchdog_stats;
}

void watchdog_list(void) {
    const char *state_names[] = {"DISABLED", "ENABLED", "EXPIRED", "TRIGGERED"};

    kprint("\n[WATCHDOG] Watchdog List:\n");
    kprint("─────────────────────────────────────────────────────────\n");

    for (u32 i = 0; i < watchdog_count; i++) {
        watchdog_t *wd = &watchdogs[i];

        kprint("  WD");
        kprinthex(wd->id);
        kprint(": ");
        kprint(wd->name);
        kprint(" | Agent ");
        kprinthex(wd->agent_id);
        kprint(" | ");
        kprinthex((u32)wd->timeout_ms);
        kprint("ms | ");
        kprint(state_names[wd->state]);
        kprint("\n");
    }

    kprint("─────────────────────────────────────────────────────────\n");
    kprint("Total: ");
    kprinthex(watchdog_count);
    kprint(" | Active: ");
    kprinthex(watchdog_stats.active_watchdogs);
    kprint(" | Expirations: ");
    kprinthex(watchdog_stats.total_expirations);
    kprint("\n\n");
}

isa_error_t watchdog_delete(u32 watchdog_id) {
    if (watchdog_id >= watchdog_count) {
        return ISA_ERR_NOT_FOUND;
    }

    watchdog_t *wd = &watchdogs[watchdog_id];

    if (wd->state == WATCHDOG_ENABLED) {
        watchdog_stats.active_watchdogs--;
    }

    wd->state = WATCHDOG_DISABLED;

    return ISA_OK;
}
