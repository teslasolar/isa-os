/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-FS: ISA Filesystem (isaf.c)
 * Hierarchical filesystem organized by ISA-95 levels
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"

// ── Filesystem State ──
#define ISA_FS_MAGIC 0x46415349  // 'ISAF'
#define ISA_FS_VERSION 1
#define BLOCK_SIZE 4096
#define MAX_FILES 1024
#define MAX_OPEN_FILES 64

static isa_superblock_t superblock;
static isa_file_t file_table[MAX_FILES];
static u32 file_count = 0;

typedef struct {
    isa_file_t *file;
    u32 position;
    bool is_open;
    isa_level_t accessor_level;
} file_descriptor_t;

static file_descriptor_t fd_table[MAX_OPEN_FILES];

// ── Helper Functions ──
static bool check_permission(isa_file_t *file, isa_level_t level) {
    // Hierarchical access: L4 can access all, L0 can only access L0
    // Each level can access its own level and levels below
    return (level >= file->owner_level);
}

static u32 hash_path(const char *path) {
    u32 hash = 0;
    while (*path) {
        hash = hash * 31 + *path++;
    }
    return hash;
}

// ────────────────────────────────────────────────────────────────────────
// Filesystem Initialization
// ────────────────────────────────────────────────────────────────────────
void isaf_init(void) {
    superblock.magic = ISA_FS_MAGIC;
    superblock.version = ISA_FS_VERSION;
    superblock.total_blocks = 1024;  // 4MB filesystem
    superblock.block_size = BLOCK_SIZE;
    superblock.inode_table = 1;      // Block 1
    superblock.data_blocks = 64;     // Block 64 onwards
    superblock.mount_level = ISA_L4_ERP; // Only L4 can mount

    // Initialize file table
    for (u32 i = 0; i < MAX_FILES; i++) {
        file_table[i].name[0] = 0;
        file_table[i].inode = 0;
    }

    // Initialize file descriptor table
    for (u32 i = 0; i < MAX_OPEN_FILES; i++) {
        fd_table[i].is_open = false;
    }

    // Create default directory structure
    const char *default_dirs[] = {
        "/L0/sensors",
        "/L0/actuators",
        "/L1/plc",
        "/L1/motion",
        "/L2/scada",
        "/L2/hmi",
        "/L3/mes",
        "/L3/batches",
        "/L4/erp",
        "/L4/reports"
    };

    for (u32 i = 0; i < sizeof(default_dirs) / sizeof(default_dirs[0]); i++) {
        // Create directory entries (simplified)
        file_count++;
    }
}

// ────────────────────────────────────────────────────────────────────────
// File Operations
// ────────────────────────────────────────────────────────────────────────

/**
 * Open a file with level-based permission check
 */
u32 isaf_open(const char *path, isa_level_t level) {
    // Find file in file table
    isa_file_t *file = 0;
    for (u32 i = 0; i < file_count; i++) {
        if (file_table[i].name[0] != 0) {
            // Simple path comparison (in real FS, would use proper lookup)
            u32 match = 1;
            for (u32 j = 0; j < ISA_FILE_NAME_MAX && path[j]; j++) {
                if (path[j] != file_table[i].name[j]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                file = &file_table[i];
                break;
            }
        }
    }

    if (!file) {
        return 0xFFFFFFFF; // File not found
    }

    // Check permissions
    if (!check_permission(file, level)) {
        return 0xFFFFFFFE; // Permission denied
    }

    // Find free file descriptor
    for (u32 fd = 0; fd < MAX_OPEN_FILES; fd++) {
        if (!fd_table[fd].is_open) {
            fd_table[fd].file = file;
            fd_table[fd].position = 0;
            fd_table[fd].is_open = true;
            fd_table[fd].accessor_level = level;
            return fd;
        }
    }

    return 0xFFFFFFFD; // No file descriptors available
}

/**
 * Read from file
 */
u32 isaf_read(u32 fd, void *buffer, u32 length) {
    if (fd >= MAX_OPEN_FILES || !fd_table[fd].is_open) {
        return 0; // Invalid FD
    }

    file_descriptor_t *desc = &fd_table[fd];
    isa_file_t *file = desc->file;

    // Bounds check
    if (desc->position >= file->size) {
        return 0; // EOF
    }

    u32 to_read = length;
    if (desc->position + to_read > file->size) {
        to_read = file->size - desc->position;
    }

    // In real implementation, would read from data blocks
    // For now, just stub
    desc->position += to_read;

    return to_read;
}

/**
 * Write to file (with audit logging)
 */
u32 isaf_write(u32 fd, const void *buffer, u32 length) {
    if (fd >= MAX_OPEN_FILES || !fd_table[fd].is_open) {
        return 0; // Invalid FD
    }

    file_descriptor_t *desc = &fd_table[fd];
    isa_file_t *file = desc->file;

    // Check write permission
    if (desc->accessor_level < file->owner_level) {
        return 0; // No write permission
    }

    // In real implementation:
    // 1. Allocate data blocks if needed
    // 2. Write data to blocks
    // 3. Update inode
    // 4. Create audit log entry (21 CFR Part 11)

    desc->position += length;
    if (desc->position > file->size) {
        file->size = desc->position;
    }

    // Update modification time
    extern u64 system_ticks;
    file->modified = system_ticks;

    return length;
}

/**
 * Close file
 */
void isaf_close(u32 fd) {
    if (fd < MAX_OPEN_FILES) {
        fd_table[fd].is_open = false;
    }
}

/**
 * Create new file
 */
u32 isaf_create(const char *path, isa_level_t owner_level, u32 perms) {
    if (file_count >= MAX_FILES) {
        return 0xFFFFFFFF; // No space
    }

    isa_file_t *file = &file_table[file_count];

    // Copy path to name
    u32 i;
    for (i = 0; i < ISA_FILE_NAME_MAX - 1 && path[i]; i++) {
        file->name[i] = path[i];
    }
    file->name[i] = 0;

    file->owner_level = owner_level;
    file->size = 0;
    file->inode = file_count + 1;
    file->perms = perms;

    extern u64 system_ticks;
    file->created = system_ticks;
    file->modified = system_ticks;

    file_count++;
    return file->inode;
}

/**
 * List files in directory (by ISA level)
 */
u32 isaf_list(isa_level_t level, isa_file_t *results, u32 max_results) {
    u32 count = 0;

    for (u32 i = 0; i < file_count && count < max_results; i++) {
        if (file_table[i].owner_level == level) {
            results[count++] = file_table[i];
        }
    }

    return count;
}
