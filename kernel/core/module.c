/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Module System (module.c)
 * Dynamic module loading and management
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/module.h"

// ── External Symbols from Linker ──
extern module_desc_t __start_modules;
extern module_desc_t __stop_modules;

// ── Module Registry ──
static module_desc_t *module_registry_head = NULL;
static u32 total_modules = 0;

// ── Helper Functions ──
static void *memset(void *dest, int val, u32 count);
static u32 strlen(const char *str);
static int strcmp(const char *s1, const char *s2);
extern void kprint(const char *str);
extern void kprinthex(u32 val);
extern u64 system_ticks;

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

static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const u8*)s1 - *(const u8*)s2;
}

// ────────────────────────────────────────────────────────────────────────
// Module System Implementation
// ────────────────────────────────────────────────────────────────────────

/**
 * Initialize module subsystem
 */
void module_system_init(void) {
    kprint("[MODULE] Initializing module system...\n");

    // Scan .modules section for statically registered modules
    module_desc_t *mod = &__start_modules;
    module_desc_t *end = &__stop_modules;

    while (mod < end) {
        if (mod->name) {
            // Add to registry
            mod->next = module_registry_head;
            module_registry_head = mod;
            total_modules++;

            kprint("[MODULE] Discovered: ");
            kprint(mod->name);
            kprint("\n");
        }
        mod++;
    }

    kprint("[MODULE] Found ");
    kprinthex(total_modules);
    kprint(" modules\n");
}

/**
 * Check if all dependencies are loaded
 */
static bool check_dependencies(module_desc_t *mod) {
    if (!mod->depends || mod->depend_count == 0) {
        return true; // No dependencies
    }

    for (u32 i = 0; i < mod->depend_count; i++) {
        const char *dep_name = mod->depends[i];
        module_desc_t *dep = module_find(dep_name);

        if (!dep || dep->status != MODULE_STATUS_RUNNING) {
            kprint("[MODULE] Dependency not loaded: ");
            kprint(dep_name);
            kprint("\n");
            return false;
        }
    }

    return true;
}

/**
 * Load specific module
 */
i32 module_load(const char *name) {
    module_desc_t *mod = module_find(name);

    if (!mod) {
        kprint("[MODULE] ERROR: Module not found: ");
        kprint(name);
        kprint("\n");
        return -1;
    }

    if (mod->status == MODULE_STATUS_RUNNING) {
        // Already loaded
        return 0;
    }

    // Check dependencies
    if (!check_dependencies(mod)) {
        kprint("[MODULE] ERROR: Dependencies not met for ");
        kprint(name);
        kprint("\n");
        return -2;
    }

    kprint("[MODULE] Loading: ");
    kprint(mod->name);
    kprint("\n");

    mod->status = MODULE_STATUS_LOADING;

    // Initialize
    if (mod->init) {
        i32 ret = mod->init();
        if (ret != 0) {
            kprint("[MODULE] ERROR: Init failed for ");
            kprint(mod->name);
            kprint("\n");
            mod->status = MODULE_STATUS_ERROR;
            return ret;
        }
    }

    mod->status = MODULE_STATUS_LOADED;

    // Start
    if (mod->start) {
        mod->start();
    }

    mod->status = MODULE_STATUS_RUNNING;
    mod->load_time = system_ticks;

    kprint("[MODULE] Started: ");
    kprint(mod->name);
    kprint("\n");

    return 0;
}

/**
 * Unload module
 */
i32 module_unload(const char *name) {
    module_desc_t *mod = module_find(name);

    if (!mod || mod->status != MODULE_STATUS_RUNNING) {
        return -1;
    }

    kprint("[MODULE] Unloading: ");
    kprint(mod->name);
    kprint("\n");

    // Stop
    if (mod->stop) {
        mod->stop();
    }

    // Cleanup
    if (mod->cleanup) {
        mod->cleanup();
    }

    mod->status = MODULE_STATUS_UNLOADED;

    return 0;
}

/**
 * Find module by name
 */
module_desc_t *module_find(const char *name) {
    module_desc_t *mod = module_registry_head;

    while (mod) {
        if (strcmp(mod->name, name) == 0) {
            return mod;
        }
        mod = mod->next;
    }

    return NULL;
}

/**
 * Auto-load all modules in dependency order
 */
void module_autoload_all(void) {
    kprint("[MODULE] Auto-loading all modules...\n");

    bool progress = true;
    u32 loaded = 0;
    u32 iterations = 0;
    const u32 max_iterations = 10;

    while (progress && iterations < max_iterations) {
        progress = false;
        iterations++;

        module_desc_t *mod = module_registry_head;
        while (mod) {
            if (mod->status == MODULE_STATUS_UNLOADED) {
                if (check_dependencies(mod)) {
                    if (module_load(mod->name) == 0) {
                        loaded++;
                        progress = true;
                    }
                }
            }
            mod = mod->next;
        }
    }

    kprint("[MODULE] Loaded ");
    kprinthex(loaded);
    kprint(" modules\n");

    // Report any unloaded modules
    module_desc_t *mod = module_registry_head;
    while (mod) {
        if (mod->status == MODULE_STATUS_UNLOADED) {
            kprint("[MODULE] WARNING: Not loaded: ");
            kprint(mod->name);
            kprint(" (missing dependencies?)\n");
        }
        mod = mod->next;
    }
}

/**
 * List all modules
 */
void module_list(void) {
    const char *type_names[] = {
        "Driver", "Protocol", "Service", "Agent", "FS", "Network", "Security", "Util"
    };

    const char *status_names[] = {
        "Unloaded", "Loading", "Loaded", "Running", "Error"
    };

    kprint("\n[MODULE] Module List:\n");
    kprint("─────────────────────────────────────────────────────\n");

    module_desc_t *mod = module_registry_head;
    u32 count = 0;

    while (mod) {
        kprint(mod->name);
        kprint(" - ");
        kprint(mod->description);
        kprint(" [");
        kprint(type_names[mod->type]);
        kprint("] (");
        kprint(status_names[mod->status]);
        kprint(")\n");

        count++;
        mod = mod->next;
    }

    kprint("─────────────────────────────────────────────────────\n");
    kprint("Total: ");
    kprinthex(count);
    kprint(" modules\n\n");
}

/**
 * Get module count
 */
u32 module_count(void) {
    return total_modules;
}

/**
 * Get modules by type
 */
u32 module_get_by_type(module_type_t type, module_desc_t **results, u32 max_results) {
    u32 count = 0;
    module_desc_t *mod = module_registry_head;

    while (mod && count < max_results) {
        if (mod->type == type) {
            results[count++] = mod;
        }
        mod = mod->next;
    }

    return count;
}
