# ISA-OS API Reference

*Auto-generated documentation for system calls and APIs*

## System Calls

### L0: Field Device I/O

#### sys_l0_read
```c
uint32_t sys_l0_read(uint32_t sensor_id, uint32_t *buffer, uint32_t unused1, uint32_t unused2);
```
**Description**: Read value from field sensor/device

**Parameters**:
- `sensor_id`: Sensor identifier (0-1023)
- `buffer`: Pointer to receive value
- `unused1`, `unused2`: Reserved

**Returns**: 0 on success, error code otherwise

**Example**:
```c
uint32_t temp;
sys_l0_read(0, &temp, 0, 0);  // Read sensor 0
```

---

#### sys_l1_write
```c
uint32_t sys_l1_write(uint32_t actuator_id, uint32_t value, uint32_t unused1, uint32_t unused2);
```
**Description**: Write value to control actuator

**Parameters**:
- `actuator_id`: Actuator identifier (0-1023)
- `value`: Value to write
- `unused1`, `unused2`: Reserved

**Returns**: 0 on success, error code otherwise

**Example**:
```c
sys_l1_write(0, 100, 0, 0);  // Set actuator 0 to 100%
```

---

### L2: SCADA Alarm Management

#### sys_l2_alarm
```c
uint32_t sys_l2_alarm(uint32_t priority, const char *msg, uint32_t unused1, uint32_t unused2);
```
**Description**: Raise ISA-18.2 compliant alarm

**Parameters**:
- `priority`: 1=Critical, 2=High, 3=Medium, 4=Low, 5=Info
- `msg`: Alarm message (null-terminated string)
- `unused1`, `unused2`: Reserved

**Returns**: 0 on success

**Example**:
```c
sys_l2_alarm(1, "CRITICAL: Temperature exceeded limit", 0, 0);
```

---

### L3: Manufacturing Operations

#### sys_l3_batch_start
```c
uint32_t sys_l3_batch_start(uint32_t recipe_id, uint32_t unused1, uint32_t unused2, uint32_t unused3);
```
**Description**: Start ISA-88 batch execution

**Parameters**:
- `recipe_id`: Recipe/formula identifier
- `unused1-3`: Reserved

**Returns**: Batch ID on success, 0xFFFFFFFF on error

**Example**:
```c
uint32_t batch_id = sys_l3_batch_start(42, 0, 0, 0);
```

---

### L4: Enterprise Integration

#### sys_l4_erp_sync
```c
uint32_t sys_l4_erp_sync(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4);
```
**Description**: Synchronize production data with ERP system

**Parameters**: All reserved

**Returns**: 0 on success

**Example**:
```c
sys_l4_erp_sync(0, 0, 0, 0);
```

---

### Agent Management

#### sys_agent_spawn
```c
uint32_t sys_agent_spawn(uint32_t level, uint32_t fn_addr, uint32_t priority, uint32_t unused);
```
**Description**: Create new ISA agent

**Parameters**:
- `level`: ISA-95 level (0-4)
- `fn_addr`: Agent entry point address
- `priority`: Scheduling priority (1-10)
- `unused`: Reserved

**Returns**: Agent ID on success, 0xFFFFFFFF on error

---

#### sys_agent_send
```c
uint32_t sys_agent_send(uint32_t dest_id, uint32_t msg, uint32_t unused1, uint32_t unused2);
```
**Description**: Send message to another agent

**Parameters**:
- `dest_id`: Destination agent ID
- `msg`: Message payload
- `unused1-2`: Reserved

**Returns**: 0 on success

---

#### sys_yield
```c
uint32_t sys_yield(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4);
```
**Description**: Yield CPU to scheduler (cooperative multitasking)

**Parameters**: All reserved

**Returns**: 0

---

## Module API

### Module Registration

```c
MODULE_REGISTER(name, description, type, level);
```

**Parameters**:
- `name`: Module name (identifier)
- `description`: Human-readable description
- `type`: MODULE_TYPE_DRIVER, MODULE_TYPE_PROTOCOL, etc.
- `level`: ISA_L0_FIELD through ISA_L4_ERP

**Example**:
```c
MODULE_REGISTER(my_sensor, "Temperature Sensor Driver", MODULE_TYPE_DRIVER, ISA_L0_FIELD);
```

---

### Module Callbacks

Every module must implement:

```c
static i32 my_module_init(void);      // Initialize resources
static void my_module_start(void);    // Start operation
static void my_module_stop(void);     // Stop operation
static void my_module_cleanup(void);  // Free resources
```

---

## Configuration API

### Get/Set Configuration

```c
void config_set_int(const char *key, i32 value);
i32 config_get_int(const char *key, i32 default_value);

void config_set_string(const char *key, const char *value);
const char *config_get_string(const char *key, const char *default_value);

void config_set_bool(const char *key, bool value);
bool config_get_bool(const char *key, bool default_value);
```

**Example**:
```c
config_set_int("system.max_agents", 512);
i32 max = config_get_int("system.max_agents", 256);
```

---

## Memory Management API

### Heap Allocation

```c
void *kmalloc(u32 size);              // Allocate memory
void kfree(void *ptr);                // Free memory
void *kcalloc(u32 num, u32 size);     // Allocate and zero
void *krealloc(void *ptr, u32 size);  // Reallocate
```

**Example**:
```c
u32 *data = kmalloc(1024);
// Use data...
kfree(data);
```

---

## Driver APIs

### CAN Bus

```c
i32 can_send(can_frame_t *frame);
i32 can_receive(can_frame_t *frame);
void can_set_filter(u32 id, u32 mask, bool extended);
```

### SPI

```c
i32 spi_transfer(u8 bus, const u8 *tx_data, u8 *rx_data, u32 length);
i32 spi_write(u8 bus, const u8 *data, u32 length);
i32 spi_read(u8 bus, u8 *data, u32 length);
void spi_configure(u8 bus, spi_config_t *config);
```

### I2C

```c
i32 i2c_write(u8 bus, u8 device_addr, const u8 *data, u32 length);
i32 i2c_read(u8 bus, u8 device_addr, u8 *data, u32 length);
i32 i2c_write_read(u8 bus, u8 device_addr, const u8 *tx_data, u32 tx_len, u8 *rx_data, u32 rx_len);
u32 i2c_scan(u8 bus, u8 *devices, u32 max_devices);
```

### RTC

```c
void rtc_get_time(datetime_t *dt);
u64 rtc_get_timestamp(void);
```

---

## Protocol APIs

### OPC UA

```c
u32 opcua_add_node(const char *browse_name, isa_level_t level, u32 initial_value);
i32 opcua_read_node(u32 node_id, u32 *value);
i32 opcua_write_node(u32 node_id, u32 value);
u32 opcua_browse(isa_level_t level, opcua_node_t *results, u32 max_results);
```

### MQTT

```c
i32 mqtt_publish(const char *topic, const u8 *payload, u32 length, u8 qos);
i32 mqtt_subscribe(const char *topic, u8 qos, void (*callback)(const char *, const u8 *, u32));
void mqtt_publish_sensor(u32 sensor_id, u32 value);
void mqtt_publish_alarm(u32 priority, const char *message);
```

---

## Watchdog API

```c
void watchdog_kick(void);                    // Reset watchdog timer
void watchdog_check(void);                   // Check for timeout
void watchdog_set_timeout(u32 timeout_ms);   // Set timeout
u32 watchdog_get_resets(void);               // Get reset count
```

---

## Shell Commands

Interactive shell (type in QEMU):

```bash
help              # Show commands
modules           # List modules
config            # Show configuration
agents            # List agents
stats             # System statistics
l0 read <id>      # Read sensor
l1 write <id> <v> # Write actuator
l2 alarm <msg>    # Raise alarm
clear             # Clear screen
reboot            # Reboot system
```

---

*For more details, see source code in `kernel/` directory*
