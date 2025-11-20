# RFC-003: ISA-SEC - Security Standard for Industrial Operating Systems

**Status:** Draft
**Author:** ISA-OS Project
**Created:** 2025-11-20
**Version:** 1.0.0

## Abstract

ISA-SEC defines comprehensive security requirements for industrial automation operating systems, addressing ICS/SCADA-specific threats while maintaining real-time performance and safety guarantees.

## Motivation

Industrial Control Systems face unique security challenges:
- Legacy protocols lack encryption (Modbus, PROFINET)
- Air-gap assumptions are obsolete (IIoT, remote access)
- Traditional IT security impacts real-time performance
- Safety and security requirements often conflict
- Incidents can cause physical harm

ISA-SEC provides defense-in-depth security without compromising real-time guarantees.

## Threat Model

### Threat Actors

1. **External Attackers:** Nation-states, hacktivists, cybercriminals
2. **Insider Threats:** Malicious or negligent employees
3. **Supply Chain:** Compromised hardware/software
4. **Physical Access:** Unauthorized access to equipment

### Attack Vectors

1. **Network:** Remote exploitation, MITM attacks
2. **Firmware:** Malicious updates, bootkit
3. **Configuration:** Malicious recipes, parameters
4. **Physical:** USB attacks, hardware tampering

### Assets to Protect

- **Safety:** Prevent physical harm (highest priority)
- **Availability:** Keep production running
- **Integrity:** Ensure correct control/data
- **Confidentiality:** Protect trade secrets (lower priority)

## Specification

### 1. Security Architecture

```
┌─────────────────────────────────────────────┐
│  L4: ERP Zone (DMZ - Limited Access)        │
├─────────────────────────────────────────────┤
│  L3: MES Zone (Restricted)                  │
├─────────────────────────────────────────────┤
│  L2: SCADA Zone (Monitored)                 │
├─────────────────────────────────────────────┤
│  L1: Control Zone (Isolated)                │
├─────────────────────────────────────────────┤
│  L0: Field Zone (Air-gap Preferred)         │
└─────────────────────────────────────────────┘
```

**Principle:** Defense-in-depth with network segmentation per ISA level

### 2. Security Levels

ISA-SEC defines security assurance levels:

| Level | Name | Requirements | Use Case |
|-------|------|--------------|----------|
| **SEC-1** | Basic | Password auth, audit logs | Non-critical systems |
| **SEC-2** | Standard | MFA, encryption, RBAC | Most industrial systems |
| **SEC-3** | Enhanced | Code signing, secure boot, HSM | Critical infrastructure |
| **SEC-4** | Safety-Critical | Formal verification, redundancy | Nuclear, chemical plants |

### 3. Core Security Requirements

#### 3.1 Secure Boot

**Requirement:** Verify integrity of bootloader and kernel

```
Boot Process:
  1. Hardware Root of Trust (ROM) validates bootloader signature
  2. Bootloader validates kernel signature
  3. Kernel validates module signatures
  4. All signatures use RSA-2048 or ECC P-256
```

**Implementation:**
```assembly
; boot/secure_boot.asm
verify_signature:
    ; Load public key from ROM (read-only)
    ; Compute SHA-256 hash of bootloader
    ; Verify RSA signature
    ; If invalid: HALT and log to TPM
```

#### 3.2 Code Signing

**Requirement:** All kernel modules must be cryptographically signed

```c
// Module signature structure
typedef struct {
    u8 signature[256];          // RSA-2048 signature
    u8 hash[32];                // SHA-256 hash
    u32 signer_id;              // Authorized signer
    u64 timestamp;              // Signing timestamp
    char signer_name[64];       // Human-readable
} module_signature_t;

// Verify module before loading
i32 module_verify_signature(module_desc_t *mod) {
    u8 computed_hash[32];
    sha256(mod->code, mod->code_size, computed_hash);

    if (memcmp(computed_hash, mod->signature.hash, 32) != 0) {
        audit_write(ISA_L0_FIELD, 0, "MODULE_TAMPERED", mod->name);
        return -1;  // REJECT
    }

    if (rsa_verify(mod->signature.signature, computed_hash, &public_key) != 0) {
        audit_write(ISA_L0_FIELD, 0, "MODULE_INVALID_SIG", mod->name);
        return -1;  // REJECT
    }

    return 0;  // OK
}
```

#### 3.3 Memory Protection

**Requirement:** Isolate agents and kernel using hardware memory protection

```c
// Page table entry with NX (No eXecute) bit
typedef struct {
    u32 present : 1;
    u32 rw : 1;           // Read/Write
    u32 user : 1;         // User/Supervisor
    u32 pwt : 1;
    u32 pcd : 1;
    u32 accessed : 1;
    u32 dirty : 1;
    u32 pat : 1;
    u32 global : 1;
    u32 avail : 3;
    u32 frame : 20;       // Physical frame
    u32 nx : 1;           // No Execute (bit 63 in 64-bit)
} page_table_entry_t;

// Memory layout per agent
Agent Memory:
  0x00000000 - 0x003FFFFF : Code (R-X, no write)
  0x00400000 - 0x007FFFFF : Data (RW-, no execute)
  0x00800000 - 0x00BFFFFF : Stack (RW-, no execute, guard page)
  0xC0000000 - 0xFFFFFFFF : Kernel (supervisor only)
```

#### 3.4 Access Control

**Requirement:** Role-Based Access Control (RBAC) with ISA-level awareness

```c
// User roles
typedef enum {
    ROLE_OPERATOR,       // View, acknowledge alarms
    ROLE_SUPERVISOR,     // Adjust setpoints, start/stop
    ROLE_ENGINEER,       // Modify recipes, upload code
    ROLE_QUALITY,        // View batch records, export data
    ROLE_ADMIN           // Full system access
} user_role_t;

// Permission matrix
typedef struct {
    user_role_t role;
    isa_level_t max_level;    // Highest level accessible
    u32 permissions;          // Bitmask of allowed operations
} permission_t;

#define PERM_READ       (1 << 0)
#define PERM_WRITE      (1 << 1)
#define PERM_EXECUTE    (1 << 2)
#define PERM_DELETE     (1 << 3)
#define PERM_ADMIN      (1 << 4)

// Permission check
bool check_permission(user_t *user, isa_level_t level, u32 operation) {
    permission_t *perm = &role_permissions[user->role];

    // Can't access levels above role's max
    if (level > perm->max_level) {
        audit_write(level, user->id, "ACCESS_DENIED", "Level too high");
        return false;
    }

    // Check if operation is permitted
    if (!(perm->permissions & operation)) {
        audit_write(level, user->id, "ACCESS_DENIED", "Operation not allowed");
        return false;
    }

    return true;
}
```

#### 3.5 Cryptography

**Requirement:** Use FIPS 140-2 validated cryptographic primitives

**Algorithms:**
- **Hashing:** SHA-256, SHA-3 (NOT MD5, SHA-1)
- **Symmetric:** AES-256-GCM (NOT DES, 3DES)
- **Asymmetric:** RSA-2048, ECC P-256 (NOT RSA-1024)
- **Key Exchange:** ECDH, X25519
- **Signatures:** RSA-PSS, ECDSA

**Key Management:**
```c
// Hardware Security Module (HSM) integration
typedef struct {
    u32 key_id;
    key_type_t type;         // RSA, ECC, AES
    u32 bits;                // Key length
    bool exportable;         // Can key leave HSM?
    u64 created;
    u64 expires;
} key_descriptor_t;

// All keys stored in HSM or TPM
i32 hsm_sign(u32 key_id, const u8 *data, u32 len, u8 *signature);
i32 hsm_verify(u32 key_id, const u8 *data, u32 len, const u8 *signature);
i32 hsm_encrypt(u32 key_id, const u8 *plaintext, u32 len, u8 *ciphertext);
i32 hsm_decrypt(u32 key_id, const u8 *ciphertext, u32 len, u8 *plaintext);
```

#### 3.6 Network Security

**Requirement:** Encrypt and authenticate all network traffic

**Protocol Security:**

| Protocol | Security | Mechanism |
|----------|----------|-----------|
| Modbus TCP | TLS 1.3 | Modbus/TCP + TLS wrapper |
| OPC UA | UA Security | Sign & Encrypt mode |
| MQTT | TLS 1.3 | MQTT over TLS (port 8883) |
| HTTP | HTTPS | TLS 1.3 only |
| SSH | SSH v2 | Key-based auth only |

**Implementation:**
```c
// TLS wrapper for Modbus TCP
typedef struct {
    tls_context_t *tls;
    modbus_context_t *modbus;
} secure_modbus_t;

i32 secure_modbus_connect(secure_modbus_t *ctx, const char *host, u16 port) {
    // 1. Establish TCP connection
    // 2. TLS handshake (verify server cert)
    // 3. Initialize Modbus protocol
    // 4. All data encrypted with AES-256-GCM
}
```

#### 3.7 Audit & Monitoring

**Requirement:** Log all security-relevant events with tamper-proof storage

```c
// Security event types
typedef enum {
    SEC_EVENT_LOGIN_SUCCESS,
    SEC_EVENT_LOGIN_FAILED,
    SEC_EVENT_PERMISSION_DENIED,
    SEC_EVENT_CONFIG_CHANGED,
    SEC_EVENT_FIRMWARE_UPDATED,
    SEC_EVENT_MODULE_LOADED,
    SEC_EVENT_CERTIFICATE_EXPIRED,
    SEC_EVENT_INTRUSION_DETECTED,
    SEC_EVENT_ANOMALY_DETECTED
} security_event_t;

// Tamper-evident logging (blockchain-style)
typedef struct {
    u64 sequence_number;     // Strictly increasing
    u64 timestamp;           // NTP-synced time
    security_event_t event;
    u32 user_id;
    isa_level_t level;
    char details[256];
    u8 prev_hash[32];        // SHA-256 of previous log
    u8 this_hash[32];        // SHA-256 of this log
    u8 signature[256];       // RSA signature (HSM)
} security_log_t;

// Verify log chain integrity
bool verify_log_chain(security_log_t *logs, u32 count) {
    for (u32 i = 1; i < count; i++) {
        u8 computed_hash[32];
        sha256(&logs[i-1], sizeof(security_log_t) - 32, computed_hash);

        if (memcmp(computed_hash, logs[i].prev_hash, 32) != 0) {
            return false;  // TAMPERED!
        }
    }
    return true;
}
```

### 4. Intrusion Detection

**Requirement:** Detect and respond to security incidents in real-time

```c
// Anomaly detection rules
typedef struct {
    char name[64];
    bool (*detect)(void);           // Detection function
    severity_t severity;            // CRITICAL, HIGH, MEDIUM, LOW
    void (*response)(void);         // Automatic response
} ids_rule_t;

// Example rules
bool detect_brute_force(void) {
    // More than 3 failed logins in 1 minute
    return (failed_login_count > 3 && elapsed_time < 60000);
}

bool detect_privilege_escalation(void) {
    // Operator trying to access L4
    return (current_user->role == ROLE_OPERATOR &&
            attempted_level == ISA_L4_ERP);
}

bool detect_abnormal_traffic(void) {
    // Unusual network patterns (ML-based)
    return ml_anomaly_score > threshold;
}

// Automated responses
void response_lock_account(void) {
    lock_user_account(current_user->id, 3600);  // 1 hour
    send_alert_to_admin("Account locked due to brute force");
}

void response_isolate_agent(void) {
    agent_suspend(suspicious_agent_id);
    network_isolate(suspicious_agent_id);
    raise_alarm(ALARM_CRITICAL, "Agent isolated due to security threat");
}
```

### 5. Incident Response

**Requirement:** Automated containment and recovery procedures

```c
// Incident response playbook
typedef struct {
    threat_type_t threat;
    containment_action_t containment;
    recovery_action_t recovery;
    u32 max_response_time_ms;
} incident_playbook_t;

// Example playbooks
incident_playbook_t playbooks[] = {
    {
        .threat = THREAT_RANSOMWARE,
        .containment = ISOLATE_NETWORK | SNAPSHOT_STATE,
        .recovery = RESTORE_FROM_BACKUP,
        .max_response_time_ms = 1000  // 1 second
    },
    {
        .threat = THREAT_PRIVILEGE_ESCALATION,
        .containment = TERMINATE_AGENT | LOCK_ACCOUNT,
        .recovery = AUDIT_REVIEW,
        .max_response_time_ms = 100  // 100ms
    }
};
```

### 6. Secure Update Mechanism

**Requirement:** Over-the-air updates must be authenticated and atomic

```c
// Firmware update package
typedef struct {
    u32 version;                    // New version number
    u32 size;                       // Update size
    u8 hash[32];                    // SHA-256 of update
    u8 signature[256];              // RSA signature
    u32 signer_id;                  // Who signed this
    u64 timestamp;                  // When signed
    char changelog[512];            // What changed
} firmware_update_t;

// Atomic update process
i32 firmware_update(firmware_update_t *update) {
    // 1. Verify signature
    if (verify_signature(update) != 0) {
        return -1;  // REJECT
    }

    // 2. Download to staging area
    download_firmware(update);

    // 3. Verify checksum
    if (verify_checksum(update) != 0) {
        return -1;  // CORRUPT
    }

    // 4. Create backup of current firmware
    backup_current_firmware();

    // 5. Apply update atomically (commit or rollback)
    if (apply_update(update) != 0) {
        rollback_firmware();  // Automatic rollback
        return -1;  // FAILED
    }

    // 6. Reboot with watchdog (auto-rollback if boot fails)
    set_boot_watchdog(30);  // 30 seconds to boot
    reboot();

    return 0;
}
```

### 7. Physical Security Integration

**Requirement:** Detect and respond to physical tampering

```c
// Physical security sensors
typedef struct {
    bool chassis_opened;         // Case opened
    bool tamper_detected;        // Physical tamper
    bool temperature_high;       // Overheating (possible attack)
    bool power_anomaly;          // Power glitching attack
} physical_security_t;

// Response to physical breach
void handle_physical_breach(void) {
    // 1. Immediately lock system
    system_lock();

    // 2. Zeroize sensitive keys
    hsm_zeroize_keys();

    // 3. Log event (if possible)
    audit_write(ISA_L0_FIELD, 0, "PHYSICAL_BREACH", "Chassis opened");

    // 4. Send alert (if network available)
    send_alert("Physical security breach detected");

    // 5. Safe shutdown (move process to safe state)
    emergency_shutdown();
}
```

### 8. Compliance Mapping

ISA-SEC aligns with existing standards:

| Standard | Scope | ISA-SEC Level |
|----------|-------|---------------|
| IEC 62443 | Industrial automation security | SEC-2/3 |
| NIST CSF | Cybersecurity framework | SEC-2 |
| ISO 27001 | Information security | SEC-2 |
| NERC CIP | Critical infrastructure | SEC-3 |
| IEC 61508 | Functional safety | SEC-3/4 |

## Testing & Validation

### Penetration Testing

Required tests:
1. **Network:** Port scanning, MITM, replay attacks
2. **Authentication:** Brute force, credential stuffing
3. **Authorization:** Privilege escalation, path traversal
4. **Injection:** Command injection, code injection
5. **Physical:** USB attacks, JTAG access
6. **Fuzzing:** Malformed packets, invalid inputs

### Security Audit Checklist

- [ ] All modules signed with valid certificates
- [ ] Secure boot enabled and tested
- [ ] Memory protection active (NX, ASLR, stack canaries)
- [ ] All network traffic encrypted (TLS 1.3)
- [ ] Audit logs tamper-evident and backed up
- [ ] Intrusion detection rules tested
- [ ] Incident response playbooks documented
- [ ] Vulnerability scan passed (<CVSS 7.0)
- [ ] Penetration test passed (0 critical findings)

## Performance Impact

ISA-SEC security features have minimal real-time impact:

| Feature | Overhead | RT Impact |
|---------|----------|-----------|
| Secure Boot | One-time (boot) | None |
| Code Signing | Per-module load | None |
| Memory Protection | <1% CPU | Negligible |
| TLS Encryption | 10-20μs/packet | Acceptable for L2+ |
| Audit Logging | 2-5μs/event | Negligible |
| IDS | 1-2% CPU | Background |

**Note:** L0/L1 field I/O does NOT use encryption to maintain hard real-time guarantees.

## Implementation

ISA-OS implements ISA-SEC in:
- `boot/secure_boot.asm` - Secure boot
- `kernel/security/` - Core security subsystem
- `kernel/crypto/` - Cryptographic primitives
- `kernel/ids/` - Intrusion detection

## References

- IEC 62443: Industrial Automation Security
- NIST SP 800-82: Guide to ICS Security
- ISA/IEC 62443: Security for Industrial Automation
- NERC CIP: Critical Infrastructure Protection
- OWASP ICS Top 10

---

**Status: DRAFT - Seeking feedback from ICS security community**
