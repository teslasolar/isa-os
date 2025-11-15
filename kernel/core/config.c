/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Configuration System (config.c)
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/config.h"
#include "../include/module.h"

#define MAX_CONFIG_ENTRIES 128

static config_entry_t config_table[MAX_CONFIG_ENTRIES];
static u32 config_count = 0;

// Helper functions
static void *memset(void *dest, int val, u32 count);
static u32 strlen(const char *str);
static void strcpy(char *dest, const char *src);
static int strcmp(const char *s1, const char *s2);
extern void kprint(const char *str);
extern void kprinthex(u32 val);

static void *memset(void *dest, int val, u32 count) {
    u8 *d = (u8 *)dest;
    while (count--) *d++ = (u8)val;
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

static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const u8*)s1 - *(const u8*)s2;
}

// ────────────────────────────────────────────────────────────────────────
// Configuration Implementation
// ────────────────────────────────────────────────────────────────────────

static i32 config_system_init(void) {
    memset(config_table, 0, sizeof(config_table));
    config_count = 0;

    // Load defaults
    config_set_int("system.scheduler_hz", DEFAULT_SCHEDULER_HZ);
    config_set_int("system.max_agents", DEFAULT_MAX_AGENTS);
    config_set_int("network.uart_baud", DEFAULT_UART_BAUD);
    config_set_int("network.can_baud", DEFAULT_CAN_BAUD);
    config_set_int("network.spi_clock", DEFAULT_SPI_CLOCK);
    config_set_int("network.i2c_clock", DEFAULT_I2C_CLOCK);
    config_set_string("network.mqtt_broker", DEFAULT_MQTT_BROKER);
    config_set_int("network.mqtt_port", DEFAULT_MQTT_PORT);
    config_set_int("network.opcua_port", DEFAULT_OPCUA_PORT);
    config_set_int("L2.alarm_buffer_size", DEFAULT_ALARM_BUFFER_SIZE);
    config_set_int("L3.audit_log_size", DEFAULT_AUDIT_LOG_SIZE);

    kprint("[CONFIG] Loaded defaults\n");
    return 0;
}

static void config_system_start(void) {}
static void config_system_stop(void) {}
static void config_system_cleanup(void) {}

/**
 * Find configuration entry by key
 */
static config_entry_t *config_find(const char *key) {
    for (u32 i = 0; i < config_count; i++) {
        if (strcmp(config_table[i].key, key) == 0) {
            return &config_table[i];
        }
    }
    return NULL;
}

/**
 * Set integer configuration
 */
void config_set_int(const char *key, i32 value) {
    config_entry_t *entry = config_find(key);

    if (!entry) {
        if (config_count >= MAX_CONFIG_ENTRIES) return;
        entry = &config_table[config_count++];
        entry->key = key;
        entry->type = CONFIG_TYPE_INT;
    }

    entry->value.int_val = value;
}

/**
 * Get integer configuration
 */
i32 config_get_int(const char *key, i32 default_value) {
    config_entry_t *entry = config_find(key);

    if (entry && entry->type == CONFIG_TYPE_INT) {
        return entry->value.int_val;
    }

    return default_value;
}

/**
 * Set string configuration
 */
void config_set_string(const char *key, const char *value) {
    config_entry_t *entry = config_find(key);

    if (!entry) {
        if (config_count >= MAX_CONFIG_ENTRIES) return;
        entry = &config_table[config_count++];
        entry->key = key;
        entry->type = CONFIG_TYPE_STRING;
    }

    u32 i;
    for (i = 0; i < 127 && value[i]; i++) {
        entry->value.str_val[i] = value[i];
    }
    entry->value.str_val[i] = 0;
}

/**
 * Get string configuration
 */
const char *config_get_string(const char *key, const char *default_value) {
    config_entry_t *entry = config_find(key);

    if (entry && entry->type == CONFIG_TYPE_STRING) {
        return entry->value.str_val;
    }

    return default_value;
}

/**
 * Set boolean configuration
 */
void config_set_bool(const char *key, bool value) {
    config_entry_t *entry = config_find(key);

    if (!entry) {
        if (config_count >= MAX_CONFIG_ENTRIES) return;
        entry = &config_table[config_count++];
        entry->key = key;
        entry->type = CONFIG_TYPE_BOOL;
    }

    entry->value.bool_val = value;
}

/**
 * Get boolean configuration
 */
bool config_get_bool(const char *key, bool default_value) {
    config_entry_t *entry = config_find(key);

    if (entry && entry->type == CONFIG_TYPE_BOOL) {
        return entry->value.bool_val;
    }

    return default_value;
}

/**
 * List all configuration entries
 */
void config_list(void) {
    kprint("\n[CONFIG] Configuration:\n");
    kprint("─────────────────────────────────────────────────────\n");

    for (u32 i = 0; i < config_count; i++) {
        config_entry_t *entry = &config_table[i];

        kprint(entry->key);
        kprint(" = ");

        switch (entry->type) {
            case CONFIG_TYPE_INT:
                kprinthex(entry->value.int_val);
                break;
            case CONFIG_TYPE_STRING:
                kprint(entry->value.str_val);
                break;
            case CONFIG_TYPE_BOOL:
                kprint(entry->value.bool_val ? "true" : "false");
                break;
            default:
                kprint("(unknown)");
        }

        kprint("\n");
    }

    kprint("─────────────────────────────────────────────────────\n\n");
}

/**
 * Save configuration (stub)
 */
i32 config_save(const char *filename) {
    // Would write to ISA-FS
    (void)filename;
    return 0;
}

/**
 * Load configuration (stub)
 */
i32 config_load(const char *filename) {
    // Would read from ISA-FS
    (void)filename;
    return 0;
}

// ── Module Registration ──
MODULE_REGISTER(config_system, "Configuration System", MODULE_TYPE_SERVICE, ISA_L0_FIELD);
