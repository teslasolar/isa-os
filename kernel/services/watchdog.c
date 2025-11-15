/**
 * ════════════════════════════════════════════════════════════════════════
 * Watchdog Timer (watchdog.c)
 * System watchdog for fault detection and recovery
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"
#include "../include/module.h"

#define WATCHDOG_TIMEOUT_MS 5000  // 5 seconds default

static bool watchdog_enabled = false;
static u64 watchdog_last_kick = 0;
static u32 watchdog_timeout_ms = WATCHDOG_TIMEOUT_MS;
static u32 watchdog_resets = 0;

// ────────────────────────────────────────────────────────────────────────
// Watchdog Functions
// ────────────────────────────────────────────────────────────────────────

static i32 watchdog_init(void) {
    extern void kprint(const char *);
    kprint("[WATCHDOG] Initialized (5s timeout)\n");
    return 0;
}

static void watchdog_start(void) {
    extern u64 system_ticks;
    watchdog_enabled = true;
    watchdog_last_kick = system_ticks;
}

static void watchdog_stop(void) {
    watchdog_enabled = false;
}

static void watchdog_cleanup(void) {}

/**
 * Kick the watchdog (reset timer)
 */
void watchdog_kick(void) {
    extern u64 system_ticks;
    watchdog_last_kick = system_ticks;
}

/**
 * Check if watchdog has expired
 */
void watchdog_check(void) {
    if (!watchdog_enabled) return;

    extern u64 system_ticks;
    u64 elapsed = system_ticks - watchdog_last_kick;

    if (elapsed > watchdog_timeout_ms) {
        // Watchdog expired!
        extern void kprint(const char *);
        extern void kprinthex(u32);

        kprint("\n[WATCHDOG] TIMEOUT! System not responding\n");
        kprint("[WATCHDOG] Elapsed: ");
        kprinthex((u32)elapsed);
        kprint(" ms\n");
        kprint("[WATCHDOG] Initiating system reset...\n");

        watchdog_resets++;

        // Reset system (triple fault method)
        __asm__ __volatile__(
            "lidt 0\n"
            "int $0xFF\n"
        );
    }
}

/**
 * Set watchdog timeout
 */
void watchdog_set_timeout(u32 timeout_ms) {
    watchdog_timeout_ms = timeout_ms;
}

/**
 * Get watchdog statistics
 */
u32 watchdog_get_resets(void) {
    return watchdog_resets;
}

// ── Module Registration ──
MODULE_REGISTER(watchdog, "Watchdog Timer", MODULE_TYPE_SERVICE, ISA_L0_FIELD);
