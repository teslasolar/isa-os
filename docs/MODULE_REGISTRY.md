# Module Registry

*Auto-discovered: 11 modules*


## DRIVERS

### can_bus
- **Description**: CAN Bus Driver
- **Level**: Core
- **File**: `kernel/drivers/can_bus.c`

### i2c
- **Description**: I2C Driver
- **Level**: Core
- **File**: `kernel/drivers/i2c.c`

### my_module
- **Description**: Description
- **Level**: Core
- **File**: `kernel/include/module.h`

### rtc
- **Description**: Real-Time Clock
- **Level**: Core
- **File**: `kernel/drivers/rtc.c`

### spi
- **Description**: SPI Driver
- **Level**: Core
- **File**: `kernel/drivers/spi.c`


## PROTOCOLS

### mqtt
- **Description**: MQTT Client
- **Level**: L2
- **File**: `kernel/services/mqtt.c`

### opc_ua
- **Description**: OPC UA Server
- **Level**: L2
- **File**: `kernel/services/opc_ua.c`


## SERVICES

### config_system
- **Description**: Configuration System
- **Level**: Core
- **File**: `kernel/core/config.c`

### memory
- **Description**: Memory Manager
- **Level**: Core
- **File**: `kernel/core/memory.c`

### shell
- **Description**: Interactive Shell
- **Level**: Core
- **File**: `kernel/services/shell.c`

### watchdog
- **Description**: Watchdog Timer
- **Level**: Core
- **File**: `kernel/services/watchdog.c`
