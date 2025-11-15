/**
 * ════════════════════════════════════════════════════════════════════════
 * Memory Manager (memory.c)
 * Simple heap allocator for kernel
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"
#include "../include/module.h"

// ── Memory Block Structure ──
typedef struct mem_block {
    u32 size;                   // Size of block (excluding header)
    bool free;                  // Is this block free?
    struct mem_block *next;     // Next block in list
} mem_block_t;

#define HEAP_SIZE (1024 * 1024)  // 1MB heap
#define BLOCK_HEADER_SIZE sizeof(mem_block_t)

static u8 heap[HEAP_SIZE] __attribute__((aligned(16)));
static mem_block_t *heap_start = NULL;
static u32 total_allocated = 0;
static u32 total_freed = 0;

// ────────────────────────────────────────────────────────────────────────
// Memory Management Functions
// ────────────────────────────────────────────────────────────────────────

static i32 memory_init(void) {
    // Initialize heap with one large free block
    heap_start = (mem_block_t *)heap;
    heap_start->size = HEAP_SIZE - BLOCK_HEADER_SIZE;
    heap_start->free = true;
    heap_start->next = NULL;

    total_allocated = 0;
    total_freed = 0;

    extern void kprint(const char *);
    kprint("[MEMORY] Heap initialized: 1MB\n");

    return 0;
}

static void memory_start(void) {}
static void memory_stop(void) {}
static void memory_cleanup(void) {}

/**
 * Allocate memory from heap
 */
void *kmalloc(u32 size) {
    if (size == 0) return NULL;

    // Align to 16 bytes
    size = (size + 15) & ~15;

    mem_block_t *current = heap_start;

    while (current) {
        if (current->free && current->size >= size) {
            // Found a suitable block

            // Split block if there's enough space left
            if (current->size >= size + BLOCK_HEADER_SIZE + 16) {
                mem_block_t *new_block = (mem_block_t *)((u8 *)current + BLOCK_HEADER_SIZE + size);
                new_block->size = current->size - size - BLOCK_HEADER_SIZE;
                new_block->free = true;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->free = false;
            total_allocated += size;

            return (void *)((u8 *)current + BLOCK_HEADER_SIZE);
        }

        current = current->next;
    }

    return NULL; // No suitable block found
}

/**
 * Free allocated memory
 */
void kfree(void *ptr) {
    if (!ptr) return;

    mem_block_t *block = (mem_block_t *)((u8 *)ptr - BLOCK_HEADER_SIZE);
    block->free = true;
    total_freed += block->size;

    // Coalesce adjacent free blocks
    mem_block_t *current = heap_start;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += BLOCK_HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

/**
 * Allocate and zero memory
 */
void *kcalloc(u32 num, u32 size) {
    u32 total = num * size;
    void *ptr = kmalloc(total);

    if (ptr) {
        u8 *p = (u8 *)ptr;
        for (u32 i = 0; i < total; i++) {
            p[i] = 0;
        }
    }

    return ptr;
}

/**
 * Reallocate memory
 */
void *krealloc(void *ptr, u32 new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    mem_block_t *block = (mem_block_t *)((u8 *)ptr - BLOCK_HEADER_SIZE);

    if (block->size >= new_size) {
        return ptr; // Current block is large enough
    }

    // Allocate new block
    void *new_ptr = kmalloc(new_size);
    if (!new_ptr) return NULL;

    // Copy old data
    u8 *src = (u8 *)ptr;
    u8 *dst = (u8 *)new_ptr;
    for (u32 i = 0; i < block->size; i++) {
        dst[i] = src[i];
    }

    kfree(ptr);
    return new_ptr;
}

/**
 * Get memory statistics
 */
void kmem_stats(u32 *allocated, u32 *freed, u32 *available) {
    *allocated = total_allocated;
    *freed = total_freed;

    u32 free_space = 0;
    mem_block_t *current = heap_start;
    while (current) {
        if (current->free) {
            free_space += current->size;
        }
        current = current->next;
    }
    *available = free_space;
}

// ── Module Registration ──
MODULE_REGISTER(memory, "Memory Manager", MODULE_TYPE_SERVICE, ISA_L0_FIELD);
