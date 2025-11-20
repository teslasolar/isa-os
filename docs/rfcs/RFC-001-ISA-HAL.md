# RFC-001: ISA-HAL - Hardware Abstraction Layer Standard

**Status:** Draft
**Author:** ISA-OS Project
**Created:** 2025-11-20
**Version:** 1.0.0

## Abstract

This RFC proposes ISA-HAL (ISA Hardware Abstraction Layer), a standardized interface for industrial automation hardware that provides uniform access to field devices, control systems, and industrial protocols across ISA-95 levels L0 and L1.

## Motivation

Current industrial automation systems lack a standardized hardware abstraction layer, leading to:
- Tight coupling between control logic and hardware
- Difficulty in testing (no hardware mocking)
- Vendor lock-in
- Complex driver development
- No hot-plug support

ISA-HAL addresses these issues by defining a standard interface that separates hardware concerns from control logic.

## Specification

### 1. Architecture Layers

```
┌─────────────────────────────────────────┐
│  L1: Control Logic (PLC, Motion, PID)   │
├─────────────────────────────────────────┤
│  ISA-HAL: Hardware Abstraction Layer    │
├─────────────────────────────────────────┤
│  L0: Protocol Drivers                   │
│  (Modbus, PROFINET, EtherCAT, etc.)     │
├─────────────────────────────────────────┤
│  Physical Hardware                      │
└─────────────────────────────────────────┘
```

### 2. Device Classes

ISA-HAL defines standard device classes:

| Class | Description | Examples |
|-------|-------------|----------|
| ANALOG_IN | Analog input (0-10V, 4-20mA) | Temperature, Pressure, Flow |
| ANALOG_OUT | Analog output | Valve position, Speed control |
| DIGITAL_IN | Digital input | Limit switches, Buttons |
| DIGITAL_OUT | Digital output (relay/transistor) | Solenoid valves, Motors |
| COUNTER | Counter/Encoder | Pulse counting, RPM |
| PWM | Pulse Width Modulation | Motor speed, Heater |
| FIELDBUS | Fieldbus devices | Modbus, PROFINET, EtherCAT nodes |

### 3. Core Data Structures

```c
// Device capabilities
typedef struct {
    u32 resolution;        // Bits (8, 10, 12, 16, 24)
    u32 sample_rate_hz;    // Max sampling rate
    float min_value;       // Physical minimum
    float max_value;       // Physical maximum
    char unit[16];         // "degC", "bar", "m3/h"
    bool hot_pluggable;    // Can be added/removed at runtime
} hal_capability_t;

// Device descriptor
typedef struct {
    u32 device_id;                   // Unique device ID
    char name[32];                   // Human-readable name
    hal_device_class_t class;        // Device class
    isa_level_t level;               // ISA level (L0 or L1)
    hal_capability_t capabilities;   // Device capabilities

    // Function pointers (driver interface)
    i32 (*init)(void);
    i32 (*read)(u32 channel, void *buffer, u32 size);
    i32 (*write)(u32 channel, const void *buffer, u32 size);
    i32 (*ioctl)(u32 cmd, void *arg);
    void (*cleanup)(void);

    // Status
    hal_device_state_t state;        // ONLINE, OFFLINE, FAULT
    u64 last_update;                 // Last successful operation
    u32 error_count;                 // Cumulative errors

    void *driver_data;               // Driver-specific data
} hal_device_t;
```

### 4. Standard Operations

#### 4.1 Device Registration

```c
// Register device with HAL
i32 hal_register_device(hal_device_t *device);

// Unregister device
i32 hal_unregister_device(u32 device_id);

// Enumerate devices
u32 hal_enumerate(hal_device_class_t class,
                  hal_device_t **devices,
                  u32 max_devices);
```

#### 4.2 Device Access

```c
// Open device for access
hal_handle_t hal_open(u32 device_id, u32 flags);

// Read from device (blocks until data available)
i32 hal_read(hal_handle_t handle, void *buffer, u32 size);

// Write to device
i32 hal_write(hal_handle_t handle, const void *buffer, u32 size);

// Control device (ioctl-style)
i32 hal_ioctl(hal_handle_t handle, u32 cmd, void *arg);

// Close device
i32 hal_close(hal_handle_t handle);
```

#### 4.3 Async/Event-Driven Access

```c
// Register callback for device events
i32 hal_register_callback(hal_handle_t handle,
                          hal_event_t event_type,
                          void (*callback)(void *data),
                          void *user_data);

// Events
typedef enum {
    HAL_EVENT_DATA_READY,     // New data available
    HAL_EVENT_THRESHOLD,      // Value crossed threshold
    HAL_EVENT_FAULT,          // Device fault
    HAL_EVENT_ONLINE,         // Device came online
    HAL_EVENT_OFFLINE         // Device went offline
} hal_event_t;
```

### 5. Error Handling

All HAL functions return standardized error codes:

```c
typedef enum {
    HAL_OK = 0,
    HAL_ERR_NOT_FOUND = -1,      // Device not found
    HAL_ERR_BUSY = -2,            // Device busy
    HAL_ERR_TIMEOUT = -3,         // Operation timeout
    HAL_ERR_IO = -4,              // I/O error
    HAL_ERR_INVALID_PARAM = -5,   // Invalid parameter
    HAL_ERR_NOT_SUPPORTED = -6,   // Operation not supported
    HAL_ERR_HARDWARE = -7,        // Hardware fault
    HAL_ERR_PERMISSION = -8       // Permission denied
} hal_error_t;
```

### 6. Real-Time Guarantees

ISA-HAL provides real-time guarantees for L0/L1 operations:

| Operation | Max Latency | Guarantee Level |
|-----------|-------------|-----------------|
| hal_read() | <10μs | Hard (L0) |
| hal_write() | <10μs | Hard (L0) |
| hal_ioctl() | <100μs | Soft (L1) |
| Event callback | <5μs | Hard (L0) |

### 7. Example: Modbus Device Driver

```c
// Modbus driver implementing ISA-HAL
static i32 modbus_init(void) {
    uart_init(9600, UART_8N1);
    return HAL_OK;
}

static i32 modbus_read(u32 channel, void *buffer, u32 size) {
    u16 *data = (u16 *)buffer;
    return modbus_read_holding_register(1, channel, data);
}

static i32 modbus_write(u32 channel, const void *buffer, u32 size) {
    u16 value = *(u16 *)buffer;
    return modbus_write_register(1, channel, value);
}

// Register Modbus device
hal_device_t modbus_device = {
    .device_id = 0x1000,
    .name = "Modbus RTU Slave 1",
    .class = HAL_DEVICE_FIELDBUS,
    .level = ISA_L0_FIELD,
    .init = modbus_init,
    .read = modbus_read,
    .write = modbus_write,
    .capabilities = {
        .resolution = 16,
        .sample_rate_hz = 100,
        .hot_pluggable = false
    }
};

hal_register_device(&modbus_device);
```

### 8. Testing & Validation

ISA-HAL enables hardware mocking for testing:

```c
// Mock device for testing
hal_device_t mock_sensor = {
    .device_id = 0xFFFF0001,
    .name = "Mock Temperature Sensor",
    .class = HAL_DEVICE_ANALOG_IN,
    .read = mock_read,  // Returns simulated data
    .write = NULL
};
```

## Benefits

1. **Portability**: Control logic is hardware-agnostic
2. **Testability**: Mock devices for unit testing
3. **Hot-plug**: Add/remove devices at runtime
4. **Diagnostics**: Uniform device health monitoring
5. **Security**: Centralized access control per device

## Implementation

ISA-OS implements ISA-HAL in `kernel/hal/` with drivers in `kernel/drivers/`.

## References

- ISA-95: Enterprise-Control System Integration
- POSIX Device I/O
- Linux Device Driver Model
- VxWorks I/O System

## Open Questions

1. Should ISA-HAL support DMA directly or leave to drivers?
2. How to handle multi-channel devices (e.g., 16-channel ADC)?
3. Should we support device composition (virtual devices)?

## Appendix A: Device Class Registry

Assigned device class IDs:

```
0x0000-0x00FF : Analog Input
0x0100-0x01FF : Analog Output
0x0200-0x02FF : Digital Input
0x0300-0x03FF : Digital Output
0x0400-0x04FF : Counters/Encoders
0x0500-0x05FF : PWM
0x1000-0x1FFF : Fieldbus (Modbus, PROFINET, EtherCAT)
0x2000-0x2FFF : Communication (Serial, Ethernet, CAN)
0x3000-0x3FFF : Motion Control
0xF000-0xFFFF : Vendor-specific
```

---

**Status: DRAFT - Seeking feedback from industrial automation community**
