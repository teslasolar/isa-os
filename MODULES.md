# ISA-OS Module System

## Overview

ISA-OS features a modular architecture with automatic module discovery and loading. Modules are registered at compile time using linker sections and automatically initialized at boot.

## Module Types

| Type       | Description                          | Examples                      |
|------------|--------------------------------------|-------------------------------|
| Driver     | Hardware drivers                     | CAN, SPI, I2C, Modbus         |
| Protocol   | Communication protocols              | OPC UA, MQTT                  |
| Service    | System services                      | Config, Audit, Scheduler      |
| Agent      | ISA automation agents                | L0 Monitor, L1 PID, L3 Batch  |
| FS         | Filesystem implementations           | ISA-FS, FAT32                 |
| Network    | Network stack components             | TCP/IP, Ethernet              |
| Security   | Security features                    | Encryption, Authentication    |
| Util       | Utilities                            | Math, String, Debug           |

## Available Modules

### Drivers (L0)

#### ✅ Modbus RTU
- **File**: `kernel/drivers/modbus.c`
- **Description**: RS-485 serial communication
- **Features**:
  - 9600 baud, 8N1
  - Read/Write holding registers
  - CRC16 checksums
  - RTU binary protocol

#### ✅ PROFINET IO
- **File**: `kernel/drivers/profinet.c`
- **Description**: Industrial Ethernet (Siemens)
- **Features**:
  - Real-time (RT/IRT)
  - DCP discovery
  - Cyclic I/O
  - Alarm handling

#### ✅ EtherCAT
- **File**: `kernel/drivers/ethercat.c`
- **Description**: High-speed industrial Ethernet
- **Features**:
  - Distributed clocks
  - Sub-millisecond cycles
  - State machine
  - Daisy-chain topology

#### ✅ CAN Bus
- **File**: `kernel/drivers/can_bus.c`
- **Description**: Controller Area Network
- **Features**:
  - 11-bit and 29-bit IDs
  - Configurable baudrate (125k-1M)
  - Hardware filters
  - TX/RX mailboxes

#### ✅ SPI
- **File**: `kernel/drivers/spi.c`
- **Description**: Serial Peripheral Interface
- **Features**:
  - Configurable clock speed
  - 4 SPI modes (CPOL/CPHA)
  - Full-duplex transfer
  - Multiple chip selects

#### ✅ I2C
- **File**: `kernel/drivers/i2c.c`
- **Description**: Inter-Integrated Circuit
- **Features**:
  - Standard (100kHz) and Fast (400kHz) modes
  - 7-bit and 10-bit addressing
  - Write/Read operations
  - Bus scanning

### Protocols (L2)

#### ✅ OPC UA
- **File**: `kernel/services/opc_ua.c`
- **Description**: OPC Unified Architecture server
- **Features**:
  - Server on port 4840
  - Node browsing by ISA level
  - Read/Write operations
  - Namespace management

#### ✅ MQTT
- **File**: `kernel/services/mqtt.c`
- **Description**: MQTT client for IIoT
- **Features**:
  - Publish/Subscribe
  - QoS 0, 1, 2
  - Topic-based routing
  - Sensor/Alarm publishing

### Services

#### ✅ Configuration System
- **File**: `kernel/core/config.c`
- **Description**: Runtime configuration
- **Features**:
  - Key-value store
  - Multiple data types (int, string, bool, float)
  - Default values
  - Save/Load (ISA-FS integration)

#### ✅ ISA-FS
- **File**: `kernel/core/isaf.c`
- **Description**: ISA-95 hierarchical filesystem
- **Features**:
  - Level-based organization
  - Permission model
  - File operations
  - Access control

#### ✅ Audit System
- **File**: `kernel/core/audit.c`
- **Description**: Compliance logging
- **Features**:
  - 21 CFR Part 11 compliant
  - CRC64 integrity
  - WHO/WHAT/WHEN tracking
  - Export functionality

## Creating a New Module

### Step 1: Define Module Functions

```c
// my_module.c
#include "../include/module.h"

static i32 my_module_init(void) {
    // Initialize resources
    return 0; // Success
}

static void my_module_start(void) {
    // Start operation
}

static void my_module_stop(void) {
    // Stop operation
}

static void my_module_cleanup(void) {
    // Free resources
}

// Your module functions here
void my_module_do_something(void) {
    // Implementation
}
```

### Step 2: Register Module

```c
// At the end of my_module.c
MODULE_REGISTER(
    my_module,                      // Name
    "My Module Description",        // Description
    MODULE_TYPE_DRIVER,             // Type
    ISA_L0_FIELD                    // ISA Level
);
```

### Step 3: Add to Makefile

```makefile
KERNEL_C = ... \
           $(KERNEL_DIR)/drivers/my_module.c
```

### Step 4: Build and Run

```bash
make clean
make
make run
```

The module will be automatically discovered and loaded at boot!

## Module with Dependencies

If your module depends on other modules:

```c
static const char *deps[] = {"config_system", "i2c"};

MODULE_REGISTER_WITH_DEPS(
    my_sensor,
    "Sensor Driver",
    MODULE_TYPE_DRIVER,
    ISA_L0_FIELD,
    deps,
    2  // Number of dependencies
);
```

The module system will ensure dependencies are loaded first.

## Module Lifecycle

```
Unloaded
    ↓
  init()    ← Initialize resources
    ↓
 Loaded
    ↓
 start()    ← Begin operation
    ↓
 Running    ← Normal operation
    ↓
  stop()    ← Cease operation
    ↓
cleanup()   ← Free resources
    ↓
Unloaded
```

## Runtime Module Management

### From Kernel

```c
// Load specific module
module_load("can_bus");

// Unload module
module_unload("can_bus");

// Find module
module_desc_t *mod = module_find("opc_ua");

// List all modules
module_list();

// Get modules by type
module_desc_t *drivers[16];
u32 count = module_get_by_type(MODULE_TYPE_DRIVER, drivers, 16);
```

### Boot Process

```
1. module_system_init()     ← Scan .modules section
2. module_autoload_all()    ← Load all modules (with deps)
3. Modules running          ← System ready
```

## Module Index

### Current Modules (11)

| Module         | Type     | Level | Status | File                         |
|----------------|----------|-------|--------|------------------------------|
| modbus         | Driver   | L0    | ✅     | kernel/drivers/modbus.c      |
| profinet       | Driver   | L0    | ✅     | kernel/drivers/profinet.c    |
| ethercat       | Driver   | L0    | ✅     | kernel/drivers/ethercat.c    |
| can_bus        | Driver   | L0    | ✅     | kernel/drivers/can_bus.c     |
| spi            | Driver   | L0    | ✅     | kernel/drivers/spi.c         |
| i2c            | Driver   | L0    | ✅     | kernel/drivers/i2c.c         |
| opc_ua         | Protocol | L2    | ✅     | kernel/services/opc_ua.c     |
| mqtt           | Protocol | L2    | ✅     | kernel/services/mqtt.c       |
| config_system  | Service  | -     | ✅     | kernel/core/config.c         |
| isaf           | FS       | -     | ⚠️     | kernel/core/isaf.c (manual)  |
| audit          | Service  | -     | ⚠️     | kernel/core/audit.c (manual) |

⚠️ = Needs MODULE_REGISTER added

## Adding More Modules - Ideas

### Drivers
- [ ] **USB Host Controller** - USB device support
- [ ] **Ethernet PHY** - Network interface
- [ ] **UART** - General serial port
- [ ] **PWM** - Pulse width modulation
- [ ] **ADC** - Analog to digital converter
- [ ] **DAC** - Digital to analog converter
- [ ] **GPIO** - General purpose I/O
- [ ] **DMA** - Direct memory access

### Protocols
- [ ] **TCP/IP Stack** - Network communication
- [ ] **HTTP Server** - Web interface
- [ ] **WebSocket** - Real-time web comms
- [ ] **CoAP** - Constrained application protocol
- [ ] **AMQP** - Advanced message queuing
- [ ] **DDS** - Data distribution service

### Services
- [ ] **Time Sync (NTP)** - Network time protocol
- [ ] **Data Logger** - Trend recording
- [ ] **Historian** - Time-series database
- [ ] **Recipe Manager** - ISA-88 recipes
- [ ] **Alarm Manager** - ISA-18.2 alarms (enhanced)
- [ ] **User Manager** - Authentication/authorization

### ISA-95 Agents
- [ ] **L0 Data Collector** - Auto-collect sensor data
- [ ] **L1 Ladder Logic** - PLC programming
- [ ] **L2 HMI Server** - Human-machine interface
- [ ] **L3 Production Tracker** - Track batches/lots
- [ ] **L4 MRP Integration** - Material requirements

## Quick Reference

### Module Registration
```c
MODULE_REGISTER(name, desc, type, level);
```

### Load Module
```c
module_load("module_name");
```

### List Modules
```c
module_list();
```

### Configuration
```c
config_set_int("key", value);
i32 val = config_get_int("key", default);
```

## See Also

- `kernel/include/module.h` - Module system API
- `kernel/core/module.c` - Module system implementation
- `kernel/include/config.h` - Configuration system API
- `kernel/linker.ld` - Linker script (`.modules` section)
