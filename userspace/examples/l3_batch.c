/**
 * ════════════════════════════════════════════════════════════════════════
 * L3 ISA-88 Batch Executor (l3_batch.c)
 * Executes batch recipes with phase management
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../lib/libisa.h"

typedef enum {
    PHASE_IDLE = 0,
    PHASE_CHARGE = 1,
    PHASE_HEAT = 2,
    PHASE_REACT = 3,
    PHASE_DISCHARGE = 4,
    PHASE_COMPLETE = 5,
    PHASE_ABORTED = 6
} batch_phase_t;

#define RECIPE_ID 42

#define VALVE_INLET     0
#define VALVE_OUTLET    1
#define HEATER          2
#define AGITATOR        3

void l3_batch_main(void) {
    batch_phase_t phase = PHASE_IDLE;
    uint32_t batch_id;

    while (1) {
        switch (phase) {
            case PHASE_IDLE:
                // Wait for batch start command
                isa_l2_alarm(5, "Batch: Waiting for start");
                batch_id = isa_l3_batch_start(RECIPE_ID);
                if (batch_id != 0xFFFFFFFF) {
                    phase = PHASE_CHARGE;
                    isa_l2_alarm(5, "Batch: Started");
                }
                break;

            case PHASE_CHARGE:
                // Open inlet valve to charge reactor
                isa_l2_alarm(5, "Batch: Charging");
                isa_l1_write(VALVE_INLET, 100);  // Fully open

                // Wait for charge complete (simplified)
                for (uint32_t i = 0; i < 1000; i++) {
                    isa_yield();
                }

                isa_l1_write(VALVE_INLET, 0);    // Close valve
                phase = PHASE_HEAT;
                break;

            case PHASE_HEAT:
                // Heat to reaction temperature
                isa_l2_alarm(5, "Batch: Heating");
                isa_l1_write(HEATER, 100);       // Max heat
                isa_l1_write(AGITATOR, 50);      // Stir at 50%

                // Wait for temperature (simplified)
                for (uint32_t i = 0; i < 2000; i++) {
                    isa_yield();
                }

                phase = PHASE_REACT;
                break;

            case PHASE_REACT:
                // Hold reaction conditions
                isa_l2_alarm(5, "Batch: Reacting");
                isa_l1_write(HEATER, 50);        // Maintain temp

                // Wait for reaction complete
                for (uint32_t i = 0; i < 5000; i++) {
                    isa_yield();
                }

                isa_l1_write(HEATER, 0);         // Turn off heat
                phase = PHASE_DISCHARGE;
                break;

            case PHASE_DISCHARGE:
                // Discharge product
                isa_l2_alarm(5, "Batch: Discharging");
                isa_l1_write(AGITATOR, 0);       // Stop agitator
                isa_l1_write(VALVE_OUTLET, 100); // Open outlet

                // Wait for discharge complete
                for (uint32_t i = 0; i < 1000; i++) {
                    isa_yield();
                }

                isa_l1_write(VALVE_OUTLET, 0);   // Close outlet
                phase = PHASE_COMPLETE;
                break;

            case PHASE_COMPLETE:
                isa_l2_alarm(5, "Batch: Complete");
                // Sync with ERP
                isa_l4_erp_sync();
                phase = PHASE_IDLE;
                break;

            case PHASE_ABORTED:
                isa_l2_alarm(1, "Batch: ABORTED");
                // Emergency shutdown
                isa_l1_write(VALVE_INLET, 0);
                isa_l1_write(VALVE_OUTLET, 0);
                isa_l1_write(HEATER, 0);
                isa_l1_write(AGITATOR, 0);
                phase = PHASE_IDLE;
                break;
        }

        isa_yield();
    }
}
