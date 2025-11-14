/**
 * ════════════════════════════════════════════════════════════════════════
 * L1 PID Controller (l1_pid.c)
 * Proportional-Integral-Derivative control loop
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../lib/libisa.h"

#define SENSOR_ID       0
#define ACTUATOR_ID     0

// PID tuning parameters
#define KP  1.2f        // Proportional gain
#define KI  0.5f        // Integral gain
#define KD  0.1f        // Derivative gain
#define DT  0.1f        // Time step (100ms)

// Setpoint
#define SETPOINT 50.0f  // Target value

void l1_pid_main(void) {
    float error = 0.0f;
    float error_integral = 0.0f;
    float error_derivative = 0.0f;
    float error_previous = 0.0f;

    while (1) {
        // Read process variable
        uint32_t pv_raw;
        isa_l0_read(SENSOR_ID, &pv_raw);
        float pv = (float)pv_raw;

        // Calculate error
        error = SETPOINT - pv;

        // Integral term (with anti-windup)
        error_integral += error * DT;
        if (error_integral > 100.0f) error_integral = 100.0f;
        if (error_integral < -100.0f) error_integral = -100.0f;

        // Derivative term
        error_derivative = (error - error_previous) / DT;

        // PID output (control variable)
        float cv = KP * error + KI * error_integral + KD * error_derivative;

        // Clamp output to valid range (0-100%)
        if (cv < 0.0f) cv = 0.0f;
        if (cv > 100.0f) cv = 100.0f;

        // Write to actuator
        isa_l1_write(ACTUATOR_ID, (uint32_t)cv);

        // Save error for next iteration
        error_previous = error;

        // Yield to scheduler
        isa_yield();
    }
}
