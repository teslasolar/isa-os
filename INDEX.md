# ISA-OS Index

Complete index of all files, modules, and features in ISA-OS.

## 📂 Directory Structure

```
isa-os/
├── boot/                       # Bootloader
│   └── boot.asm               # 16-bit → 32-bit bootloader
├── kernel/                     # Kernel source
│   ├── core/                  # Core kernel components
│   │   ├── kernel.asm         # Assembly entry, syscalls
│   │   ├── kernel.c           # Main kernel, scheduler
│   │   ├── module.c           # Module system
│   │   ├── config.c           # Configuration system
│   │   ├── isaf.c             # ISA-FS filesystem
│   │   └── audit.c            # Audit logging
│   ├── drivers/               # Hardware drivers (L0)
│   │   ├── modbus.c           # Modbus RTU
│   │   ├── profinet.c         # PROFINET IO
│   │   ├── ethercat.c         # EtherCAT
│   │   ├── can_bus.c          # CAN Bus
│   │   ├── spi.c              # SPI
│   │   └── i2c.c              # I2C
│   ├── services/              # System services (L2+)
│   │   ├── opc_ua.c           # OPC UA server
│   │   └── mqtt.c             # MQTT client
│   ├── include/               # Header files
│   │   ├── isa_types.h        # Type definitions
│   │   ├── module.h           # Module system API
│   │   └── config.h           # Configuration API
│   └── linker.ld              # Linker script
├── userspace/                  # Userspace programs
│   ├── lib/
│   │   └── libisa.h           # Syscall library
│   └── examples/
│       ├── l0_monitor.c       # L0 sensor monitor
│       ├── l1_pid.c           # L1 PID controller
│       └── l3_batch.c         # L3 batch executor
├── config/                     # Configuration files
│   ├── isa-os.yaml            # Main config (YAML)
│   ├── isa-os.json            # Main config (JSON)
│   └── modules.conf           # Module config
├── build/                      # Build output (generated)
├── docs/                       # Documentation
├── .isa-os.manifest           # System manifest
├── ARCHITECTURE.md            # Architecture docs
├── MODULES.md                 # Module system docs
├── INDEX.md                   # This file
├── README.md                  # Main readme
├── LICENSE                    # MIT License
├── Makefile                   # Build system
└── .gitignore                 # Git ignore rules
```

## 🧬 Core Components

### Bootloader
- **boot/boot.asm** (272 lines)
  - Real mode startup
  - GDT setup
  - A20 line enable
  - Protected mode transition
  - Kernel loading

### Kernel Core
- **kernel/core/kernel.asm** (247 lines)
  - Kernel entry point
  - IDT setup
  - Timer interrupt (PIT)
  - Syscall handler (INT 0x80)
  - Context switching

- **kernel/core/kernel.c** (391 lines)
  - Main kernel (`kmain`)
  - ISA-95 syscalls (L0-L4)
  - Agent scheduler
  - IPC system
  - VGA console

- **kernel/core/module.c** (232 lines)
  - Module discovery
  - Auto-loading
  - Dependency resolution
  - Runtime management

- **kernel/core/config.c** (187 lines)
  - Configuration store
  - Get/Set operations
  - Default values
  - ISA-FS integration

- **kernel/core/isaf.c** (223 lines)
  - ISA filesystem
  - Level-based permissions
  - File operations
  - Directory structure

- **kernel/core/audit.c** (188 lines)
  - Audit logging
  - CRC64 integrity
  - Compliance (21 CFR Part 11)
  - Export functionality

## 🔌 Drivers (L0)

| Driver     | File                  | Lines | Features                        |
|------------|-----------------------|-------|---------------------------------|
| Modbus RTU | drivers/modbus.c      | 157   | RS-485, CRC16, Read/Write regs  |
| PROFINET   | drivers/profinet.c    | 109   | DCP, RT/IRT, Cyclic I/O         |
| EtherCAT   | drivers/ethercat.c    | 123   | Distributed clocks, State machine |
| CAN Bus    | drivers/can_bus.c     | 98    | 11/29-bit IDs, Filters          |
| SPI        | drivers/spi.c         | 107   | 4 modes, Full-duplex            |
| I2C        | drivers/i2c.c         | 125   | 100/400kHz, Bus scanning        |

## 📡 Protocols (L2)

| Protocol | File               | Lines | Features                    |
|----------|--------------------|-------|-----------------------------|
| OPC UA   | services/opc_ua.c  | 134   | Server, Nodes, Browse       |
| MQTT     | services/mqtt.c    | 145   | Pub/Sub, QoS 0/1/2, Topics  |

## 📊 Statistics

### Code Metrics
```
Total Lines of Code: ~3,800
  - Assembly:        ~520  (13.7%)
  - C:              ~3,100 (81.6%)
  - Headers:         ~180  (4.7%)

Total Files: 32
  - Source (.c/.asm): 19
  - Headers (.h):      3
  - Config:            4
  - Documentation:     6

Modules: 12
  - Drivers:    6
  - Protocols:  2
  - Services:   4

ISA Levels Implemented:
  - L0: Field I/O ✅
  - L1: Control ✅
  - L2: SCADA ✅
  - L3: MES ✅
  - L4: ERP ✅
```

### File Sizes
```
boot.asm         ~  8 KB
kernel.c         ~ 15 KB
module.c         ~  9 KB
config.c         ~  7 KB
isaf.c           ~  8 KB
audit.c          ~  7 KB
All drivers      ~ 18 KB
All services     ~ 10 KB
Total source     ~ 82 KB
```

## 🎯 ISA-95 Syscalls

| Level | Syscall              | Number | Function                  |
|-------|----------------------|--------|---------------------------|
| L0    | sys_l0_read          | 0      | Read sensor               |
| L1    | sys_l1_write         | 1      | Write actuator            |
| L2    | sys_l2_alarm         | 2      | Raise alarm               |
| L3    | sys_l3_batch_start   | 3      | Start batch               |
| L4    | sys_l4_erp_sync      | 4      | Sync with ERP             |
| Agent | sys_agent_spawn      | 5      | Create agent              |
| Agent | sys_agent_send       | 6      | Send message              |
| Agent | sys_yield            | 7      | Yield CPU                 |

## 🔧 Configuration Files

### isa-os.yaml (Main Configuration)
- System settings
- ISA level configuration
- Protocol settings
- Driver configuration
- Security & compliance
- Network settings

### isa-os.json (Alternative Format)
- Same structure as YAML
- JSON format for tools
- Machine-readable

### modules.conf (Module Loading)
- Module enable/disable
- Load order
- Driver parameters
- Protocol settings

### .isa-os.manifest (System Manifest)
- Module inventory
- Version information
- Standards compliance
- Capabilities
- Checksums

## 📖 Documentation

| File             | Purpose                              |
|------------------|--------------------------------------|
| README.md        | Overview, quick start, examples      |
| ARCHITECTURE.md  | Technical architecture, internals    |
| MODULES.md       | Module system, creating modules      |
| INDEX.md         | This file - complete index           |
| LICENSE          | MIT License                          |

## 🚀 Build Targets

```bash
make              # Build everything
make clean        # Clean build artifacts
make run          # Run in QEMU
make debug        # Debug with GDB
make help         # Show help
```

## 📦 Build Artifacts

Generated in `build/`:
- boot.bin        # Bootloader (512 bytes)
- kernel.bin      # Kernel binary
- isa-os.img      # Bootable image (1.44MB floppy)
- *.o             # Object files

## 🔍 Search Index

### By Feature
- **Modular System**: module.c, module.h, MODULES.md
- **Configuration**: config.c, config.h, config/*.yaml
- **ISA-95**: isa_types.h, kernel.c (all syscalls)
- **ISA-88**: kernel.c (batch system), l3_batch.c
- **ISA-18.2**: kernel.c (alarms), l0_monitor.c
- **21 CFR Part 11**: audit.c
- **Real-Time**: kernel.c (scheduler), kernel.asm (timer)
- **IPC**: kernel.c (message_queue)
- **Filesystem**: isaf.c

### By ISA Level
- **L0**: modbus.c, profinet.c, ethercat.c, can_bus.c, spi.c, i2c.c
- **L1**: kernel.c (sys_l1_write), l1_pid.c
- **L2**: opc_ua.c, mqtt.c, kernel.c (sys_l2_alarm)
- **L3**: kernel.c (sys_l3_batch_start), l3_batch.c
- **L4**: kernel.c (sys_l4_erp_sync)

### By Protocol
- **Modbus**: drivers/modbus.c
- **PROFINET**: drivers/profinet.c
- **EtherCAT**: drivers/ethercat.c
- **CAN**: drivers/can_bus.c
- **OPC UA**: services/opc_ua.c
- **MQTT**: services/mqtt.c

## 🎓 Learning Path

1. **Start Here**: README.md
2. **Understand Architecture**: ARCHITECTURE.md
3. **See Bootloader**: boot/boot.asm
4. **Study Kernel**: kernel/core/kernel.c
5. **Learn Modules**: MODULES.md
6. **Try Examples**: userspace/examples/
7. **Add Features**: Follow MODULES.md guide

## 🔗 Quick Links

- Main README: [README.md](README.md)
- Architecture: [ARCHITECTURE.md](ARCHITECTURE.md)
- Module System: [MODULES.md](MODULES.md)
- Configuration: [config/isa-os.yaml](config/isa-os.yaml)
- Manifest: [.isa-os.manifest](.isa-os.manifest)

---

**ISA-OS** - Industrial Automation Operating System
**Version**: 1.0.0
**License**: MIT
**Architecture**: x86_32 (with plans for x86_64, ARM, RISC-V)
