/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Userspace Library (libisa.h)
 * System call wrappers for userspace applications
 * ════════════════════════════════════════════════════════════════════════
 */

#ifndef LIBISA_H
#define LIBISA_H

#include <stdint.h>
#include <stdbool.h>

// ── ISA-95 Level Definitions ──
typedef enum {
    ISA_L0_FIELD = 0,
    ISA_L1_CONTROL = 1,
    ISA_L2_SUPER = 2,
    ISA_L3_MES = 3,
    ISA_L4_ERP = 4
} isa_level_t;

// ────────────────────────────────────────────────────────────────────────
// L0: Field Device I/O
// ────────────────────────────────────────────────────────────────────────

/**
 * Read from sensor/field device
 * @param sensor_id Device ID (0-1023)
 * @param buffer Pointer to receive value
 * @return 0 on success, error code otherwise
 */
static inline uint32_t isa_l0_read(uint32_t sensor_id, uint32_t *buffer) {
    uint32_t ret;
    __asm__ __volatile__(
        "mov $0, %%eax\n"
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"(sensor_id), "r"(buffer)
        : "eax", "ebx", "ecx"
    );
    return ret;
}

// ────────────────────────────────────────────────────────────────────────
// L1: Control Device I/O
// ────────────────────────────────────────────────────────────────────────

/**
 * Write to actuator/control device
 * @param actuator_id Device ID (0-1023)
 * @param value Value to write
 * @return 0 on success, error code otherwise
 */
static inline uint32_t isa_l1_write(uint32_t actuator_id, uint32_t value) {
    uint32_t ret;
    __asm__ __volatile__(
        "mov $1, %%eax\n"
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"(actuator_id), "r"(value)
        : "eax", "ebx", "ecx"
    );
    return ret;
}

// ────────────────────────────────────────────────────────────────────────
// L2: SCADA Alarm Management (ISA-18.2)
// ────────────────────────────────────────────────────────────────────────

/**
 * Raise an alarm
 * @param priority 1=critical, 2=high, 3=medium, 4=low, 5=info
 * @param msg Alarm message
 */
static inline void isa_l2_alarm(uint32_t priority, const char *msg) {
    __asm__ __volatile__(
        "mov $2, %%eax\n"
        "mov %0, %%ebx\n"
        "mov %1, %%ecx\n"
        "int $0x80\n"
        :
        : "r"(priority), "r"(msg)
        : "eax", "ebx", "ecx"
    );
}

// ────────────────────────────────────────────────────────────────────────
// L3: Manufacturing Operations (ISA-88)
// ────────────────────────────────────────────────────────────────────────

/**
 * Start batch execution
 * @param recipe_id Recipe/formula ID
 * @return Batch ID on success, 0xFFFFFFFF on error
 */
static inline uint32_t isa_l3_batch_start(uint32_t recipe_id) {
    uint32_t ret;
    __asm__ __volatile__(
        "mov $3, %%eax\n"
        "mov %1, %%ebx\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"(recipe_id)
        : "eax", "ebx"
    );
    return ret;
}

// ────────────────────────────────────────────────────────────────────────
// L4: Enterprise Resource Planning
// ────────────────────────────────────────────────────────────────────────

/**
 * Synchronize with ERP system
 * @return 0 on success
 */
static inline uint32_t isa_l4_erp_sync(void) {
    uint32_t ret;
    __asm__ __volatile__(
        "mov $4, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        :
        : "eax"
    );
    return ret;
}

// ────────────────────────────────────────────────────────────────────────
// Agent Management
// ────────────────────────────────────────────────────────────────────────

/**
 * Spawn new agent
 * @param level ISA-95 level for agent
 * @param fn Agent entry point function
 * @param priority Scheduling priority (1-10)
 * @return Agent ID on success, 0xFFFFFFFF on error
 */
static inline uint32_t isa_agent_spawn(isa_level_t level, void (*fn)(), uint32_t priority) {
    uint32_t ret;
    __asm__ __volatile__(
        "mov $5, %%eax\n"
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov %3, %%edx\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint32_t)level), "r"((uint32_t)fn), "r"(priority)
        : "eax", "ebx", "ecx", "edx"
    );
    return ret;
}

/**
 * Send message to another agent
 * @param dest_id Destination agent ID
 * @param msg Message payload
 */
static inline void isa_agent_send(uint32_t dest_id, uint32_t msg) {
    __asm__ __volatile__(
        "mov $6, %%eax\n"
        "mov %0, %%ebx\n"
        "mov %1, %%ecx\n"
        "int $0x80\n"
        :
        : "r"(dest_id), "r"(msg)
        : "eax", "ebx", "ecx"
    );
}

/**
 * Yield CPU to scheduler
 */
static inline void isa_yield(void) {
    __asm__ __volatile__(
        "mov $7, %%eax\n"
        "int $0x80\n"
        :
        :
        : "eax"
    );
}

#endif // LIBISA_H
