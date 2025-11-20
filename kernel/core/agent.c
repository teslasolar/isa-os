/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Agent Management (agent.c)
 * Lightweight agent system with real context switching
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"
#include "../include/error.h"

// ── External Functions ──
extern void kprint(const char *str);
extern void kprinthex(u32 val);
extern void ctx_switch(u32 **old_sp, u32 **new_sp);

// ── Agent System State ──
#define AGENT_STACK_SIZE 4096  // 4KB stack per agent

static agent_t agents[MAX_AGENTS];
static u32 agent_count = 0;
static u32 current_agent_id = 0;
static u8 agent_stacks[MAX_AGENTS][AGENT_STACK_SIZE] __attribute__((aligned(16)));

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
// Stack Setup for New Agent
// ────────────────────────────────────────────────────────────────────────

/**
 * Setup initial stack frame for agent
 * Stack layout (from top to bottom):
 *   - Return address (agent function)
 *   - EAX, EBX, ECX, EDX, ESI, EDI, EBP, EFLAGS
 */
static void agent_setup_stack(agent_t *agent, void (*entry_point)(void)) {
    // Get stack top (grows downward)
    u32 *stack = (u32 *)&agent_stacks[agent->id][AGENT_STACK_SIZE];

    // Setup initial stack frame (context_switch expects this)
    *(--stack) = 0x00000202;        // EFLAGS (IF=1, reserved bit)
    *(--stack) = 0x00000000;        // EBP
    *(--stack) = 0x00000000;        // EDI
    *(--stack) = 0x00000000;        // ESI
    *(--stack) = 0x00000000;        // EDX
    *(--stack) = 0x00000000;        // ECX
    *(--stack) = 0x00000000;        // EBX
    *(--stack) = 0x00000000;        // EAX
    *(--stack) = (u32)entry_point;  // Return address (agent starts here)

    // Set agent's stack pointer
    agent->sp = stack;
}

/**
 * Default agent entry point (wrapper)
 */
static void agent_entry_wrapper(void) {
    agent_t *agent = &agents[current_agent_id];

    kprint("[AGENT] Starting agent: ");
    kprint(agent->name);
    kprint("\n");

    // Jump to actual agent function
    if (agent->pc != 0) {
        void (*entry)(void) = (void (*)(void))agent->pc;
        entry();
    }

    // If agent returns, terminate it
    kprint("[AGENT] Agent ");
    kprint(agent->name);
    kprint(" terminated\n");

    agent->state = AGENT_TERMINATED;

    // Yield to scheduler (will not return)
    while (1) {
        __asm__ __volatile__("int $0x80" : : "a"(7));  // sys_yield
    }
}

// ────────────────────────────────────────────────────────────────────────
// Agent Management Functions
// ────────────────────────────────────────────────────────────────────────

/**
 * Initialize agent subsystem
 */
void agent_init(void) {
    memset(agents, 0, sizeof(agents));
    memset(agent_stacks, 0, sizeof(agent_stacks));
    agent_count = 0;
    current_agent_id = 0;

    kprint("[AGENT] Agent subsystem initialized\n");
}

/**
 * Create new agent
 */
u32 agent_create(isa_level_t level, u32 priority, void (*entry_point)(void), const char *name) {
    if (agent_count >= MAX_AGENTS) {
        return ISA_ERR_NO_AGENT_SLOTS;
    }

    agent_t *agent = &agents[agent_count];

    // Initialize agent structure
    agent->id = agent_count;
    agent->level = level;
    agent->priority = priority;
    agent->quantum = priority * 10;  // Priority-based time slicing
    agent->state = AGENT_READY;
    agent->pc = (u32)entry_point;
    agent->mb_head = 0;
    agent->mb_tail = 0;

    // Set name
    if (name) {
        u32 len = strlen_internal(name);
        if (len >= AGENT_NAME_MAX) len = AGENT_NAME_MAX - 1;
        for (u32 i = 0; i < len; i++) {
            agent->name[i] = name[i];
        }
        agent->name[len] = 0;
    } else {
        agent->name[0] = 'A';
        agent->name[1] = '0' + (agent_count % 10);
        agent->name[2] = 0;
    }

    // Setup stack with initial context
    agent_setup_stack(agent, agent_entry_wrapper);

    kprint("[AGENT] Created agent ");
    kprinthex(agent->id);
    kprint(": ");
    kprint(agent->name);
    kprint(" (L");
    kprinthex(level);
    kprint(", P");
    kprinthex(priority);
    kprint(")\n");

    agent_count++;
    return agent->id;
}

/**
 * Get agent by ID
 */
agent_t *agent_get(u32 agent_id) {
    if (agent_id >= agent_count) {
        return NULL;
    }
    return &agents[agent_id];
}

/**
 * Get current running agent
 */
agent_t *agent_current(void) {
    return &agents[current_agent_id];
}

/**
 * Get agent count
 */
u32 agent_get_count(void) {
    return agent_count;
}

/**
 * Terminate agent
 */
isa_error_t agent_terminate(u32 agent_id) {
    if (agent_id >= agent_count) {
        return ISA_ERR_AGENT_NOT_FOUND;
    }

    agent_t *agent = &agents[agent_id];
    agent->state = AGENT_TERMINATED;

    kprint("[AGENT] Terminated agent ");
    kprinthex(agent_id);
    kprint("\n");

    return ISA_OK;
}

/**
 * Suspend agent
 */
isa_error_t agent_suspend(u32 agent_id) {
    if (agent_id >= agent_count) {
        return ISA_ERR_AGENT_NOT_FOUND;
    }

    agent_t *agent = &agents[agent_id];
    if (agent->state == AGENT_READY || agent->state == AGENT_RUNNING) {
        agent->state = AGENT_BLOCKED;
    }

    return ISA_OK;
}

/**
 * Resume agent
 */
isa_error_t agent_resume(u32 agent_id) {
    if (agent_id >= agent_count) {
        return ISA_ERR_AGENT_NOT_FOUND;
    }

    agent_t *agent = &agents[agent_id];
    if (agent->state == AGENT_BLOCKED) {
        agent->state = AGENT_READY;
    }

    return ISA_OK;
}

// ────────────────────────────────────────────────────────────────────────
// Scheduler
// ────────────────────────────────────────────────────────────────────────

/**
 * Priority-based round-robin scheduler with real context switching
 */
void agent_schedule(void) {
    if (agent_count == 0) return;

    agent_t *current = &agents[current_agent_id];
    u32 next_id = (current_agent_id + 1) % agent_count;
    u32 attempts = 0;
    u32 highest_priority = 0;
    u32 best_agent_id = current_agent_id;

    // Find highest priority ready agent
    for (u32 i = 0; i < agent_count && attempts < agent_count; i++) {
        u32 agent_id = (current_agent_id + i + 1) % agent_count;
        agent_t *agent = &agents[agent_id];

        if (agent->state == AGENT_READY && agent->priority > highest_priority) {
            highest_priority = agent->priority;
            best_agent_id = agent_id;
        }

        attempts++;
    }

    // If found a different agent, switch context
    if (best_agent_id != current_agent_id) {
        agent_t *next = &agents[best_agent_id];

        // Update states
        if (current->state == AGENT_RUNNING) {
            current->state = AGENT_READY;
        }
        next->state = AGENT_RUNNING;

        // Perform context switch
        u32 old_id = current_agent_id;
        current_agent_id = best_agent_id;

        // Actual context switch (assembly)
        ctx_switch(&current->sp, &next->sp);

        // We return here after being rescheduled
    } else {
        // No better agent, continue current
        current->state = AGENT_RUNNING;
    }
}

/**
 * Timer tick handler (called from timer interrupt)
 */
void agent_timer_tick(void) {
    if (agent_count == 0) return;

    agent_t *current = &agents[current_agent_id];

    // Decrement quantum
    if (current->quantum > 0) {
        current->quantum--;
    }

    // If quantum expired, reset and schedule
    if (current->quantum == 0) {
        current->quantum = current->priority * 10;
        agent_schedule();
    }
}

/**
 * List all agents
 */
void agent_list(void) {
    const char *state_names[] = {"IDLE", "READY", "RUNNING", "BLOCKED", "TERM"};
    const char *level_names[] = {"L0", "L1", "L2", "L3", "L4"};

    kprint("\n[AGENT] Agent List:\n");
    kprint("─────────────────────────────────────────────────────────\n");

    for (u32 i = 0; i < agent_count; i++) {
        agent_t *agent = &agents[i];

        kprint("  ");
        kprinthex(agent->id);
        kprint(" | ");
        kprint(agent->name);
        kprint(" | ");
        kprint(level_names[agent->level]);
        kprint(" | P");
        kprinthex(agent->priority);
        kprint(" | ");
        kprint(state_names[agent->state]);
        kprint("\n");
    }

    kprint("─────────────────────────────────────────────────────────\n");
    kprint("Total: ");
    kprinthex(agent_count);
    kprint(" agents\n\n");
}
