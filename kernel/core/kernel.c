/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Kernel Core (kernel.c)
 * Main kernel implementation with ISA-95 level support
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"

// ── External Assembly Functions ──
extern void enable_interrupts(void);
extern void disable_interrupts(void);
extern void ctx_switch(u32 **old_sp, u32 **new_sp);

// ── Global State ──
static agent_t agents[MAX_AGENTS];
static u32 agent_count = 0;
static u32 current_agent = 0;
static u64 system_ticks = 0;

static isa_msg_t message_queue[MAX_MESSAGES];
static u32 msg_head = 0;
static u32 msg_tail = 0;

static isa_alarm_t alarm_queue[MAX_ALARMS];
static u32 alarm_count = 0;

static batch_t batches[MAX_BATCHES];
static u32 batch_count = 0;

// ── VGA Text Mode Output ──
static u16 *vga_buffer = (u16 *)VGA_TEXT_BASE;
static u32 vga_col = 0;
static u32 vga_row = 0;

static void kputchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else {
        u16 attr = 0x0F00; // White on black
        vga_buffer[vga_row * 80 + vga_col] = attr | c;
        vga_col++;
    }

    if (vga_col >= 80) {
        vga_col = 0;
        vga_row++;
    }

    if (vga_row >= 25) {
        vga_row = 0; // Simple wrap
    }
}

static void kprint(const char *str) {
    while (*str) {
        kputchar(*str++);
    }
}

static void kprinthex(u32 val) {
    char hex[] = "0123456789ABCDEF";
    kprint("0x");
    for (int i = 28; i >= 0; i -= 4) {
        kputchar(hex[(val >> i) & 0xF]);
    }
}

// ── String Functions ──
static void *memset(void *dest, int val, u32 count) {
    u8 *d = (u8 *)dest;
    while (count--) *d++ = (u8)val;
    return dest;
}

static void *memcpy(void *dest, const void *src, u32 count) {
    u8 *d = (u8 *)dest;
    const u8 *s = (const u8 *)src;
    while (count--) *d++ = *s++;
    return dest;
}

static u32 strlen(const char *str) {
    u32 len = 0;
    while (str[len]) len++;
    return len;
}

static void strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));
}

// ────────────────────────────────────────────────────────────────────────
// ISA-95 Level 0: Field Device I/O
// ────────────────────────────────────────────────────────────────────────
static u32 sys_l0_read(u32 sensor_id, u32 *buffer, u32 unused1, u32 unused2) {
    (void)unused1; (void)unused2;

    // Read from memory-mapped I/O
    volatile u32 *io = (volatile u32 *)MMIO_BASE;

    if (sensor_id < 1024 && buffer) {
        *buffer = io[sensor_id];
        return 0; // Success
    }

    return 1; // Error
}

static u32 sys_l1_write(u32 actuator_id, u32 value, u32 unused1, u32 unused2) {
    (void)unused1; (void)unused2;

    // Write to memory-mapped I/O
    volatile u32 *io = (volatile u32 *)MMIO_BASE;

    if (actuator_id < 1024) {
        io[actuator_id] = value;
        return 0; // Success
    }

    return 1; // Error
}

// ────────────────────────────────────────────────────────────────────────
// ISA-95 Level 2: SCADA Alarm Management (ISA-18.2)
// ────────────────────────────────────────────────────────────────────────
static u32 sys_l2_alarm(u32 priority, const char *msg, u32 unused1, u32 unused2) {
    (void)unused1; (void)unused2;

    if (alarm_count >= MAX_ALARMS) {
        return 1; // Alarm queue full
    }

    isa_alarm_t *alarm = &alarm_queue[alarm_count++];
    alarm->id = alarm_count;
    alarm->priority = (alarm_priority_t)priority;
    alarm->source_level = agents[current_agent].level;
    alarm->source_agent = current_agent;
    alarm->timestamp = system_ticks;
    alarm->acknowledged = false;

    if (msg) {
        u32 len = strlen(msg);
        if (len >= ISA_ALARM_MSG_MAX) len = ISA_ALARM_MSG_MAX - 1;
        memcpy(alarm->message, msg, len);
        alarm->message[len] = 0;
    } else {
        alarm->message[0] = 0;
    }

    // Print alarm to console
    kprint("[ALARM] P");
    kprinthex(priority);
    kprint(" L");
    kprinthex(agents[current_agent].level);
    kprint(": ");
    if (msg) kprint(msg);
    kprint("\n");

    return 0;
}

// ────────────────────────────────────────────────────────────────────────
// ISA-95 Level 3: Batch Management (ISA-88)
// ────────────────────────────────────────────────────────────────────────
static u32 sys_l3_batch_start(u32 recipe_id, u32 unused1, u32 unused2, u32 unused3) {
    (void)unused1; (void)unused2; (void)unused3;

    if (batch_count >= MAX_BATCHES) {
        return 0xFFFFFFFF; // No batch slots
    }

    batch_t *batch = &batches[batch_count];
    batch->recipe_id = recipe_id;
    batch->batch_id = batch_count;
    batch->state = PHASE_RUNNING;
    batch->start_time = system_ticks;
    batch->current_phase = 0;
    batch->total_phases = 4; // Default: Charge, Heat, React, Discharge

    kprint("[BATCH] Started recipe ");
    kprinthex(recipe_id);
    kprint("\n");

    return batch_count++;
}

// ────────────────────────────────────────────────────────────────────────
// ISA-95 Level 4: ERP Integration
// ────────────────────────────────────────────────────────────────────────
static u32 sys_l4_erp_sync(u32 unused1, u32 unused2, u32 unused3, u32 unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;

    // Stub: In real system, would sync with SAP/Oracle/etc.
    kprint("[ERP] Syncing production data\n");
    return 0;
}

// ────────────────────────────────────────────────────────────────────────
// Agent Management
// ────────────────────────────────────────────────────────────────────────
static u32 sys_agent_spawn(u32 level, u32 fn_addr, u32 priority, u32 unused) {
    (void)unused;

    if (agent_count >= MAX_AGENTS) {
        return 0xFFFFFFFF; // No agent slots
    }

    agent_t *agent = &agents[agent_count];
    agent->id = agent_count;
    agent->level = (isa_level_t)level;
    agent->pc = fn_addr;
    agent->priority = priority;
    agent->quantum = priority * 10; // Priority-based time slicing
    agent->state = AGENT_READY;
    agent->mb_head = 0;
    agent->mb_tail = 0;

    // Set default name
    const char *level_names[] = {"L0", "L1", "L2", "L3", "L4"};
    strcpy(agent->name, level_names[level]);

    kprint("[AGENT] Spawned ");
    kprint(agent->name);
    kprint(" (ID ");
    kprinthex(agent->id);
    kprint(")\n");

    return agent_count++;
}

// ────────────────────────────────────────────────────────────────────────
// IPC: Inter-Agent Messaging
// ────────────────────────────────────────────────────────────────────────
static u32 sys_agent_send(u32 dest_id, u32 msg, u32 unused1, u32 unused2) {
    (void)unused1; (void)unused2;

    if (dest_id >= agent_count) return 1;
    if (msg_tail >= MAX_MESSAGES) return 1;

    isa_msg_t *m = &message_queue[msg_tail++];
    m->src_id = current_agent;
    m->dst_id = dest_id;
    m->src_level = agents[current_agent].level;
    m->dst_level = agents[dest_id].level;
    m->msg_type = msg;
    m->timestamp = system_ticks;
    m->delivered = false;

    // Try to deliver to agent's mailbox
    agent_t *dst = &agents[dest_id];
    if (dst->mb_tail < AGENT_MAILBOX_SIZE) {
        dst->mailbox[dst->mb_tail++] = msg;
        m->delivered = true;
    }

    return 0;
}

// ────────────────────────────────────────────────────────────────────────
// Scheduler: Round-robin with priority
// ────────────────────────────────────────────────────────────────────────
static void schedule(void) {
    if (agent_count == 0) return;

    u32 next = (current_agent + 1) % agent_count;
    u32 attempts = 0;

    // Find next ready agent
    while (agents[next].state != AGENT_READY && attempts < agent_count) {
        next = (next + 1) % agent_count;
        attempts++;
    }

    if (attempts >= agent_count) {
        return; // No ready agents
    }

    if (next != current_agent) {
        agent_t *old = &agents[current_agent];
        agent_t *new = &agents[next];
        current_agent = next;

        // Context switch (in real implementation)
        // ctx_switch(&old->sp, &new->sp);
    }
}

static u32 sys_yield(u32 unused1, u32 unused2, u32 unused3, u32 unused4) {
    (void)unused1; (void)unused2; (void)unused3; (void)unused4;
    schedule();
    return 0;
}

// ────────────────────────────────────────────────────────────────────────
// Syscall Dispatcher
// ────────────────────────────────────────────────────────────────────────
typedef u32 (*syscall_fn)(u32, u32, u32, u32);

static syscall_fn syscall_table[SYS_MAX] = {
    sys_l0_read,        // 0
    sys_l1_write,       // 1
    sys_l2_alarm,       // 2
    sys_l3_batch_start, // 3
    sys_l4_erp_sync,    // 4
    sys_agent_spawn,    // 5
    sys_agent_send,     // 6
    sys_yield           // 7
};

u32 syscall_dispatcher(u32 num, u32 arg1, u32 arg2, u32 arg3, u32 arg4) {
    if (num >= SYS_MAX) {
        return 0xFFFFFFFF; // Invalid syscall
    }

    return syscall_table[num](arg1, arg2, arg3, arg4);
}

// ────────────────────────────────────────────────────────────────────────
// Timer Interrupt Handler (called from assembly)
// ────────────────────────────────────────────────────────────────────────
void timer_interrupt_handler(void) {
    system_ticks++;

    // Decrement quantum for current agent
    if (agent_count > 0 && agents[current_agent].quantum > 0) {
        agents[current_agent].quantum--;

        if (agents[current_agent].quantum == 0) {
            // Time slice expired, reset and schedule
            agents[current_agent].quantum = agents[current_agent].priority * 10;
            schedule();
        }
    }
}

// ────────────────────────────────────────────────────────────────────────
// Kernel Main Entry Point
// ────────────────────────────────────────────────────────────────────────
void kmain(void) {
    // Clear screen
    for (u32 i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = 0x0F00 | ' ';
    }

    kprint("ISA-OS Kernel v1.0\n");
    kprint("==================\n\n");

    kprint("Initializing ISA-95 Levels...\n");

    // Initialize agent system
    memset(agents, 0, sizeof(agents));
    memset(message_queue, 0, sizeof(message_queue));
    memset(alarm_queue, 0, sizeof(alarm_queue));
    memset(batches, 0, sizeof(batches));

    kprint("Agents: OK\n");
    kprint("IPC: OK\n");
    kprint("Alarms: OK\n");
    kprint("Batches: OK\n\n");

    // Spawn default agents for each ISA level
    kprint("Spawning ISA agents...\n");

    // L0: Field device monitor
    sys_agent_spawn(ISA_L0_FIELD, 0x400000, 10, 0);

    // L1: PLC controller
    sys_agent_spawn(ISA_L1_CONTROL, 0x500000, 8, 0);

    // L2: SCADA supervisor
    sys_agent_spawn(ISA_L2_SUPER, 0x600000, 5, 0);

    // L3: MES operations
    sys_agent_spawn(ISA_L3_MES, 0x700000, 3, 0);

    // L4: ERP integration
    sys_agent_spawn(ISA_L4_ERP, 0x800000, 1, 0);

    kprint("\n");
    kprint("ISA-OS Ready!\n");
    kprint("Active Agents: ");
    kprinthex(agent_count);
    kprint("\n");
    kprint("System Ticks: Running\n\n");

    // Test alarm system
    sys_l2_alarm(ALARM_INFO, "System initialized");

    // Scheduler loop
    while (1) {
        schedule();

        // Halt until next interrupt
        __asm__ __volatile__("hlt");
    }
}
