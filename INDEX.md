# ISA-OS Comprehensive Index

*Auto-generated: 2025-11-15 00:46:40*

## 📊 Statistics

```
Total Files:        26
  C source files:   20
  Assembly files:   2
  Header files:     4
Total Lines:        4,350
Registered Modules: 11
System Calls:       12
```

## 🧩 Registered Modules

| Module | Type | Level | Description | File |
|--------|------|-------|-------------|------|
| can_bus | DRIVER | Core | CAN Bus Driver | `kernel/drivers/can_bus.c` |
| config_system | SERVICE | Core | Configuration System | `kernel/core/config.c` |
| i2c | DRIVER | Core | I2C Driver | `kernel/drivers/i2c.c` |
| memory | SERVICE | Core | Memory Manager | `kernel/core/memory.c` |
| mqtt | PROTOCOL | L2 | MQTT Client | `kernel/services/mqtt.c` |
| my_module | DRIVER | Core | Description | `kernel/include/module.h` |
| opc_ua | PROTOCOL | L2 | OPC UA Server | `kernel/services/opc_ua.c` |
| rtc | DRIVER | Core | Real-Time Clock | `kernel/drivers/rtc.c` |
| shell | SERVICE | Core | Interactive Shell | `kernel/services/shell.c` |
| spi | DRIVER | Core | SPI Driver | `kernel/drivers/spi.c` |
| watchdog | SERVICE | Core | Watchdog Timer | `kernel/services/watchdog.c` |

## 🔧 System Calls

| Syscall | File |
|---------|------|
| sys_agent_send | `kernel/core/kernel.c` |
| sys_agent_spawn | `kernel/core/kernel.c` |
| sys_l0_read | `kernel/core/kernel.c` |
| sys_l0_read | `kernel/services/shell.c` |
| sys_l1_write | `kernel/core/kernel.c` |
| sys_l1_write | `kernel/services/shell.c` |
| sys_l2_alarm | `kernel/drivers/profinet.c` |
| sys_l2_alarm | `kernel/core/kernel.c` |
| sys_l2_alarm | `kernel/services/shell.c` |
| sys_l3_batch_start | `kernel/core/kernel.c` |
| sys_l4_erp_sync | `kernel/core/kernel.c` |
| sys_yield | `kernel/core/kernel.c` |

## 📂 File Tree


### boot/

- **boot.asm** (7.8 KB)

### kernel/core/

- **audit.c** (5.5 KB)
- **config.c** (6.6 KB)
- **isaf.c** (7.1 KB)
- **kernel.asm** (10.6 KB)
- **kernel.c** (14.1 KB)
- **memory.c** (4.9 KB)
- **module.c** (7.7 KB)

### kernel/drivers/

- **can_bus.c** (2.9 KB)
- **ethercat.c** (3.2 KB)
- **i2c.c** (3.0 KB)
- **modbus.c** (4.8 KB)
- **profinet.c** (3.1 KB)
- **rtc.c** (4.4 KB)
- **spi.c** (2.8 KB)

### kernel/include/

- **config.h** (2.7 KB)
- **isa_types.h** (5.7 KB)
- **module.h** (4.3 KB)

### kernel/services/

- **mqtt.c** (3.4 KB)
- **opc_ua.c** (3.4 KB)
- **shell.c** (8.0 KB)
- **watchdog.c** (2.8 KB)

### userspace/examples/

- **l0_monitor.c** (1.5 KB)
- **l1_pid.c** (1.9 KB)
- **l3_batch.c** (3.9 KB)

### userspace/lib/

- **libisa.h** (6.9 KB)

## 📚 Documentation

- [README.md](README.md) - Main overview
- [QUICKSTART.md](QUICKSTART.md) - 5-minute setup
- [ARCHITECTURE.md](ARCHITECTURE.md) - Technical architecture
- [MODULES.md](MODULES.md) - Module system guide
- [INDEX.md](INDEX.md) - This file (auto-generated)

## 🔍 Quick Search

### By Feature
- **ISA-95 Levels**: L0 (Field), L1 (Control), L2 (SCADA), L3 (MES), L4 (ERP)
- **Protocols**: Modbus, PROFINET, EtherCAT, CAN, OPC UA, MQTT
- **Interfaces**: SPI, I2C, UART
- **Standards**: ISA-95, ISA-88, ISA-18.2, 21 CFR Part 11

### By Type

**Drivers (5)**: can_bus, i2c, rtc, spi, my_module

**Protocols (2)**: mqtt, opc_ua

**Services (4)**: config_system, memory, shell, watchdog

---

*This index is automatically generated from source code.*
*Run `./scripts/generate_index.py` to update.*