/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Configuration System (config.h)
 * Runtime configuration and settings
 * ════════════════════════════════════════════════════════════════════════
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "isa_types.h"

// ── Configuration Entry ──
typedef enum {
    CONFIG_TYPE_INT = 0,
    CONFIG_TYPE_STRING = 1,
    CONFIG_TYPE_BOOL = 2,
    CONFIG_TYPE_FLOAT = 3
} config_type_t;

typedef struct {
    const char *key;
    config_type_t type;
    union {
        i32 int_val;
        char str_val[128];
        bool bool_val;
        float float_val;
    } value;
    const char *description;
} config_entry_t;

// ── Configuration Categories ──
#define CONFIG_CATEGORY_SYSTEM   "system"
#define CONFIG_CATEGORY_NETWORK  "network"
#define CONFIG_CATEGORY_L0       "L0"
#define CONFIG_CATEGORY_L1       "L1"
#define CONFIG_CATEGORY_L2       "L2"
#define CONFIG_CATEGORY_L3       "L3"
#define CONFIG_CATEGORY_L4       "L4"

// ── Configuration Functions ──

/**
 * Initialize configuration system
 */
void config_init(void);

/**
 * Set integer configuration
 */
void config_set_int(const char *key, i32 value);

/**
 * Get integer configuration
 */
i32 config_get_int(const char *key, i32 default_value);

/**
 * Set string configuration
 */
void config_set_string(const char *key, const char *value);

/**
 * Get string configuration
 */
const char *config_get_string(const char *key, const char *default_value);

/**
 * Set boolean configuration
 */
void config_set_bool(const char *key, bool value);

/**
 * Get boolean configuration
 */
bool config_get_bool(const char *key, bool default_value);

/**
 * List all configuration entries
 */
void config_list(void);

/**
 * Save configuration to file
 */
i32 config_save(const char *filename);

/**
 * Load configuration from file
 */
i32 config_load(const char *filename);

// ── Default Configuration ──
#define DEFAULT_SCHEDULER_HZ        1000    // 1ms tick
#define DEFAULT_MAX_AGENTS          256
#define DEFAULT_UART_BAUD           9600
#define DEFAULT_CAN_BAUD            500000
#define DEFAULT_SPI_CLOCK           1000000
#define DEFAULT_I2C_CLOCK           100000
#define DEFAULT_MQTT_BROKER         "mqtt.local"
#define DEFAULT_MQTT_PORT           1883
#define DEFAULT_OPCUA_PORT          4840
#define DEFAULT_ALARM_BUFFER_SIZE   128
#define DEFAULT_AUDIT_LOG_SIZE      4096

#endif // CONFIG_H
