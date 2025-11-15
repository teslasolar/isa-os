/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Module System (module.h)
 * Auto-loading module registration and initialization
 * ════════════════════════════════════════════════════════════════════════
 */

#ifndef MODULE_H
#define MODULE_H

#include "isa_types.h"

// ── Module Types ──
typedef enum {
    MODULE_TYPE_DRIVER = 0,      // Hardware driver
    MODULE_TYPE_PROTOCOL = 1,    // Communication protocol
    MODULE_TYPE_SERVICE = 2,     // System service
    MODULE_TYPE_AGENT = 3,       // ISA agent
    MODULE_TYPE_FS = 4,          // Filesystem
    MODULE_TYPE_NETWORK = 5,     // Network stack
    MODULE_TYPE_SECURITY = 6,    // Security feature
    MODULE_TYPE_UTIL = 7         // Utility
} module_type_t;

// ── Module Status ──
typedef enum {
    MODULE_STATUS_UNLOADED = 0,
    MODULE_STATUS_LOADING = 1,
    MODULE_STATUS_LOADED = 2,
    MODULE_STATUS_RUNNING = 3,
    MODULE_STATUS_ERROR = 4
} module_status_t;

// ── Module Descriptor ──
typedef struct module_desc {
    const char *name;            // Module name
    const char *description;     // Short description
    module_type_t type;          // Module type
    u32 version;                 // Version (e.g., 0x010000 = 1.0.0)
    isa_level_t level;           // ISA level (or ISA_L0_FIELD for N/A)

    // Lifecycle callbacks
    i32 (*init)(void);           // Initialize module
    void (*start)(void);         // Start module
    void (*stop)(void);          // Stop module
    void (*cleanup)(void);       // Cleanup resources

    // Dependencies
    const char **depends;        // Array of module name dependencies
    u32 depend_count;            // Number of dependencies

    // Runtime state
    module_status_t status;      // Current status
    u64 load_time;               // When module was loaded
    void *private_data;          // Module-specific data

    struct module_desc *next;    // Linked list
} module_desc_t;

// ── Module Registration Macro ──
// Usage: MODULE_REGISTER(my_module, "Description", MODULE_TYPE_DRIVER, ISA_L0_FIELD);
#define MODULE_REGISTER(name, desc, type, lvl) \
    static module_desc_t __module_##name __attribute__((used, section(".modules"))) = { \
        .name = #name, \
        .description = desc, \
        .type = type, \
        .version = 0x010000, \
        .level = lvl, \
        .init = name##_init, \
        .start = name##_start, \
        .stop = name##_stop, \
        .cleanup = name##_cleanup, \
        .depends = NULL, \
        .depend_count = 0, \
        .status = MODULE_STATUS_UNLOADED, \
        .load_time = 0, \
        .private_data = NULL, \
        .next = NULL \
    }

// Alternative with dependencies
#define MODULE_REGISTER_WITH_DEPS(name, desc, type, lvl, deps, dep_count) \
    static const char *__deps_##name[] = deps; \
    static module_desc_t __module_##name __attribute__((used, section(".modules"))) = { \
        .name = #name, \
        .description = desc, \
        .type = type, \
        .version = 0x010000, \
        .level = lvl, \
        .init = name##_init, \
        .start = name##_start, \
        .stop = name##_stop, \
        .cleanup = name##_cleanup, \
        .depends = __deps_##name, \
        .depend_count = dep_count, \
        .status = MODULE_STATUS_UNLOADED, \
        .load_time = 0, \
        .private_data = NULL, \
        .next = NULL \
    }

// ── Module System Functions ──

/**
 * Initialize module subsystem
 */
void module_system_init(void);

/**
 * Auto-discover and load all modules
 */
void module_autoload_all(void);

/**
 * Load specific module by name
 */
i32 module_load(const char *name);

/**
 * Unload module by name
 */
i32 module_unload(const char *name);

/**
 * Find module by name
 */
module_desc_t *module_find(const char *name);

/**
 * List all modules
 */
void module_list(void);

/**
 * Get module count
 */
u32 module_count(void);

/**
 * Get modules by type
 */
u32 module_get_by_type(module_type_t type, module_desc_t **results, u32 max_results);

#endif // MODULE_H
