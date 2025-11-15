/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Shell (shell.c)
 * Simple command-line interface for debugging and management
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"
#include "../include/module.h"
#include "../include/config.h"

#define CMD_BUFFER_SIZE 128
#define MAX_ARGS 8

static char cmd_buffer[CMD_BUFFER_SIZE];
static u32 cmd_pos = 0;

// Forward declarations
extern void kprint(const char *str);
extern void kprinthex(u32 val);
extern u32 agent_count;

// ────────────────────────────────────────────────────────────────────────
// Command Handlers
// ────────────────────────────────────────────────────────────────────────

static void cmd_help(void) {
    kprint("\nISA-OS Shell Commands:\n");
    kprint("  help              - Show this help\n");
    kprint("  modules           - List modules\n");
    kprint("  config            - Show configuration\n");
    kprint("  agents            - List agents\n");
    kprint("  stats             - System statistics\n");
    kprint("  l0 read <id>      - Read L0 sensor\n");
    kprint("  l1 write <id> <v> - Write L1 actuator\n");
    kprint("  l2 alarm <msg>    - Raise L2 alarm\n");
    kprint("  clear             - Clear screen\n");
    kprint("  reboot            - Reboot system\n");
    kprint("\n");
}

static void cmd_modules(void) {
    module_list();
}

static void cmd_config(void) {
    config_list();
}

static void cmd_agents(void) {
    kprint("\n[AGENTS] Active Agents:\n");
    kprint("─────────────────────────────────────────────────────\n");
    kprint("Total agents: ");
    kprinthex(agent_count);
    kprint("\n─────────────────────────────────────────────────────\n\n");
}

static void cmd_stats(void) {
    extern u64 system_ticks;
    extern u32 alarm_count;

    kprint("\n[STATS] System Statistics:\n");
    kprint("─────────────────────────────────────────────────────\n");
    kprint("System ticks: ");
    kprinthex(system_ticks);
    kprint("\nActive agents: ");
    kprinthex(agent_count);
    kprint("\nAlarms raised: ");
    kprinthex(alarm_count);
    kprint("\nModules loaded: ");
    kprinthex(module_count());
    kprint("\n─────────────────────────────────────────────────────\n\n");
}

static void cmd_clear(void) {
    extern u16 *vga_buffer;
    extern u32 vga_row, vga_col;

    for (u32 i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = 0x0F00 | ' ';
    }
    vga_row = 0;
    vga_col = 0;
}

static void cmd_l0_read(const char *arg) {
    u32 sensor_id = 0;
    // Parse sensor_id from arg (simple atoi)
    while (*arg >= '0' && *arg <= '9') {
        sensor_id = sensor_id * 10 + (*arg - '0');
        arg++;
    }

    u32 value = 0;
    extern u32 sys_l0_read(u32, u32*, u32, u32);
    sys_l0_read(sensor_id, &value, 0, 0);

    kprint("Sensor ");
    kprinthex(sensor_id);
    kprint(" = ");
    kprinthex(value);
    kprint("\n");
}

static void cmd_l1_write(const char *arg1, const char *arg2) {
    u32 actuator_id = 0, value = 0;

    while (*arg1 >= '0' && *arg1 <= '9') {
        actuator_id = actuator_id * 10 + (*arg1 - '0');
        arg1++;
    }

    while (*arg2 >= '0' && *arg2 <= '9') {
        value = value * 10 + (*arg2 - '0');
        arg2++;
    }

    extern u32 sys_l1_write(u32, u32, u32, u32);
    sys_l1_write(actuator_id, value, 0, 0);

    kprint("Actuator ");
    kprinthex(actuator_id);
    kprint(" set to ");
    kprinthex(value);
    kprint("\n");
}

static void cmd_l2_alarm(const char *msg) {
    extern u32 sys_l2_alarm(u32, const char*, u32, u32);
    sys_l2_alarm(3, msg, 0, 0);
    kprint("Alarm raised\n");
}

// ────────────────────────────────────────────────────────────────────────
// Command Parser
// ────────────────────────────────────────────────────────────────────────

static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const u8*)s1 - *(const u8*)s2;
}

static int strncmp(const char *s1, const char *s2, u32 n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return n ? (*(const u8*)s1 - *(const u8*)s2) : 0;
}

static void parse_command(const char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "modules") == 0) {
        cmd_modules();
    } else if (strcmp(cmd, "config") == 0) {
        cmd_config();
    } else if (strcmp(cmd, "agents") == 0) {
        cmd_agents();
    } else if (strcmp(cmd, "stats") == 0) {
        cmd_stats();
    } else if (strcmp(cmd, "clear") == 0) {
        cmd_clear();
    } else if (strncmp(cmd, "l0 read ", 8) == 0) {
        cmd_l0_read(cmd + 8);
    } else if (strncmp(cmd, "l1 write ", 9) == 0) {
        // Parse two arguments
        const char *arg1 = cmd + 9;
        const char *arg2 = arg1;
        while (*arg2 && *arg2 != ' ') arg2++;
        if (*arg2) arg2++;
        cmd_l1_write(arg1, arg2);
    } else if (strncmp(cmd, "l2 alarm ", 9) == 0) {
        cmd_l2_alarm(cmd + 9);
    } else if (strcmp(cmd, "reboot") == 0) {
        kprint("Rebooting...\n");
        __asm__ __volatile__("jmp 0x0000");
    } else {
        kprint("Unknown command: ");
        kprint(cmd);
        kprint("\nType 'help' for commands\n");
    }
}

// ────────────────────────────────────────────────────────────────────────
// Shell Interface
// ────────────────────────────────────────────────────────────────────────

static void shell_prompt(void) {
    kprint("isa-os> ");
}

void shell_handle_char(char c) {
    if (c == '\n' || c == '\r') {
        kprint("\n");
        if (cmd_pos > 0) {
            cmd_buffer[cmd_pos] = 0;
            parse_command(cmd_buffer);
            cmd_pos = 0;
        }
        shell_prompt();
    } else if (c == '\b' || c == 0x7F) {
        // Backspace
        if (cmd_pos > 0) {
            cmd_pos--;
            kprint("\b \b");
        }
    } else if (c >= 32 && c < 127) {
        // Printable character
        if (cmd_pos < CMD_BUFFER_SIZE - 1) {
            cmd_buffer[cmd_pos++] = c;
            char str[2] = {c, 0};
            kprint(str);
        }
    }
}

static i32 shell_init(void) {
    kprint("\n[SHELL] Initializing interactive shell...\n");
    cmd_pos = 0;
    return 0;
}

static void shell_start(void) {
    kprint("[SHELL] Type 'help' for commands\n\n");
    shell_prompt();
}

static void shell_stop(void) {
    // Nothing to stop
}

static void shell_cleanup(void) {
    // Nothing to cleanup
}

// ── Module Registration ──
MODULE_REGISTER(shell, "Interactive Shell", MODULE_TYPE_SERVICE, ISA_L0_FIELD);
