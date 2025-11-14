/**
 * ════════════════════════════════════════════════════════════════════════
 * ISA-OS Type Definitions (isa_types.h)
 * Core types for ISA-95 level architecture
 * ════════════════════════════════════════════════════════════════════════
 */

#ifndef ISA_TYPES_H
#define ISA_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// ── Basic Types ──
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

// ── ISA-95 Level Enumeration ──
typedef enum {
    ISA_L0_FIELD = 0,           // Sensors, actuators, field devices
    ISA_L1_CONTROL = 1,         // PLC, DCS, motion control
    ISA_L2_SUPER = 2,           // SCADA, HMI, supervisory
    ISA_L3_MES = 3,             // Manufacturing operations (MES)
    ISA_L4_ERP = 4,             // Enterprise planning (ERP)
    ISA_LEVEL_MAX = 5
} isa_level_t;

// ── Agent State ──
typedef enum {
    AGENT_IDLE = 0,
    AGENT_READY = 1,
    AGENT_RUNNING = 2,
    AGENT_BLOCKED = 3,
    AGENT_TERMINATED = 4
} agent_state_t;

// ── Agent Structure (lightweight process) ──
#define AGENT_NAME_MAX 32
#define AGENT_MAILBOX_SIZE 16
#define AGENT_STATE_SIZE 64

typedef struct {
    u32 id;                     // Unique agent ID
    isa_level_t level;          // ISA-95 level (L0-L4)
    u32 *sp;                    // Stack pointer
    u32 pc;                     // Program counter
    agent_state_t state;        // Current state
    u32 priority;               // Scheduling priority (1-10)
    u32 quantum;                // Time slice remaining
    char name[AGENT_NAME_MAX];  // Human-readable name
    u32 mailbox[AGENT_MAILBOX_SIZE]; // Message queue
    u32 mb_head, mb_tail;       // Mailbox indices
    u64 state_data[AGENT_STATE_SIZE]; // Agent-specific state
} agent_t;

// ── IPC Message Structure ──
#define ISA_MSG_PAYLOAD_SIZE 8

typedef struct {
    u32 src_id;                 // Source agent ID
    u32 dst_id;                 // Destination agent ID
    isa_level_t src_level;      // Source ISA level
    isa_level_t dst_level;      // Destination ISA level
    u32 msg_type;               // Message type/command
    u32 payload[ISA_MSG_PAYLOAD_SIZE]; // Message data
    u64 timestamp;              // When message was sent
    bool delivered;             // Delivery flag
} isa_msg_t;

// ── ISA-18.2 Alarm Structure ──
#define ISA_ALARM_MSG_MAX 128

typedef enum {
    ALARM_CRITICAL = 1,
    ALARM_HIGH = 2,
    ALARM_MEDIUM = 3,
    ALARM_LOW = 4,
    ALARM_INFO = 5
} alarm_priority_t;

typedef struct {
    u32 id;                     // Alarm ID
    alarm_priority_t priority;  // Alarm priority
    isa_level_t source_level;   // Which level triggered alarm
    u32 source_agent;           // Which agent triggered alarm
    u64 timestamp;              // When alarm occurred
    char message[ISA_ALARM_MSG_MAX]; // Alarm description
    bool acknowledged;          // Has operator acknowledged?
    u64 ack_timestamp;          // When acknowledged
    u32 ack_user;               // Who acknowledged
} isa_alarm_t;

// ── ISA-88 Batch Structures ──
typedef enum {
    PHASE_IDLE = 0,
    PHASE_RUNNING = 1,
    PHASE_PAUSED = 2,
    PHASE_COMPLETE = 3,
    PHASE_ABORTED = 4
} phase_state_t;

typedef struct {
    u32 recipe_id;
    u32 batch_id;
    phase_state_t state;
    u64 start_time;
    u64 end_time;
    u32 current_phase;
    u32 total_phases;
} batch_t;

// ── Audit Log (21 CFR Part 11) ──
#define AUDIT_ACTION_MAX 64
#define AUDIT_DATA_MAX 128

typedef struct {
    u64 timestamp;              // When action occurred
    isa_level_t level;          // Which level
    u32 agent_id;               // Which agent
    u32 user_id;                // Which user (if applicable)
    char action[AUDIT_ACTION_MAX];   // What was done
    char data[AUDIT_DATA_MAX];       // Data affected
    u64 crc;                    // Integrity checksum
} audit_log_t;

// ── ISA-FS File Structure ──
#define ISA_FILE_NAME_MAX 32

typedef struct {
    char name[ISA_FILE_NAME_MAX];
    isa_level_t owner_level;   // Which level owns this file
    u32 size;                   // File size in bytes
    u32 inode;                  // Inode number
    u32 perms;                  // Permission matrix (L0-L4)
    u64 created;                // Creation timestamp
    u64 modified;               // Last modified timestamp
} isa_file_t;

typedef struct {
    u32 magic;                  // 'ISAF' (0x46415349)
    u32 version;                // Filesystem version
    u32 total_blocks;           // Total blocks
    u32 block_size;             // Block size (typically 4096)
    u32 inode_table;            // Inode table location
    u32 data_blocks;            // Data blocks start
    isa_level_t mount_level;    // Which level can mount
} isa_superblock_t;

// ── Syscall Numbers ──
#define SYS_L0_READ         0
#define SYS_L1_WRITE        1
#define SYS_L2_ALARM        2
#define SYS_L3_BATCH_START  3
#define SYS_L4_ERP_SYNC     4
#define SYS_AGENT_SPAWN     5
#define SYS_AGENT_SEND      6
#define SYS_YIELD           7
#define SYS_MAX             8

// ── Constants ──
#define MAX_AGENTS          256
#define MAX_MESSAGES        1024
#define MAX_ALARMS          128
#define MAX_BATCHES         64
#define MAX_AUDIT_LOGS      4096

// ── I/O Base Addresses ──
#define MMIO_BASE           0xE0000000
#define VGA_TEXT_BASE       0xB8000

#endif // ISA_TYPES_H
