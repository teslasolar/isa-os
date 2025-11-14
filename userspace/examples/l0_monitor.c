/**
 * ════════════════════════════════════════════════════════════════════════
 * L0 Field Device Monitor (l0_monitor.c)
 * Continuously monitors sensors and triggers alarms
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../lib/libisa.h"

#define TEMP_SENSOR_ID      0
#define PRESSURE_SENSOR_ID  1
#define FLOW_SENSOR_ID      2

#define TEMP_ALARM_LIMIT    100     // °C
#define PRESSURE_ALARM_LIMIT 1000   // kPa
#define FLOW_ALARM_LIMIT    50      // L/min

void l0_monitor_main(void) {
    uint32_t temp, pressure, flow;

    while (1) {
        // Read sensors
        isa_l0_read(TEMP_SENSOR_ID, &temp);
        isa_l0_read(PRESSURE_SENSOR_ID, &pressure);
        isa_l0_read(FLOW_SENSOR_ID, &flow);

        // Check limits and raise alarms
        if (temp > TEMP_ALARM_LIMIT) {
            isa_l2_alarm(1, "CRITICAL: Temperature exceeded limit");
        }

        if (pressure > PRESSURE_ALARM_LIMIT) {
            isa_l2_alarm(2, "HIGH: Pressure exceeded limit");
        }

        if (flow < FLOW_ALARM_LIMIT) {
            isa_l2_alarm(3, "MEDIUM: Flow rate below minimum");
        }

        // Cooperative multitasking
        isa_yield();
    }
}
