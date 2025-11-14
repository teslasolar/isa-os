/**
 * ════════════════════════════════════════════════════════════════════════
 * Modbus RTU Driver (modbus.c)
 * RS-485 serial communication for industrial devices
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"

// ── UART Registers (16550 compatible) ──
#define UART_BASE       0x3F8
#define UART_DATA       (UART_BASE + 0)
#define UART_IER        (UART_BASE + 1)
#define UART_LSR        (UART_BASE + 5)
#define UART_LSR_THRE   0x20
#define UART_LSR_DR     0x01

// ── Modbus Function Codes ──
#define MODBUS_READ_COILS           0x01
#define MODBUS_READ_DISCRETE        0x02
#define MODBUS_READ_HOLDING         0x03
#define MODBUS_READ_INPUT           0x04
#define MODBUS_WRITE_COIL           0x05
#define MODBUS_WRITE_REGISTER       0x06
#define MODBUS_WRITE_MULTIPLE_COILS 0x0F
#define MODBUS_WRITE_MULTIPLE_REGS  0x10

// ── Helper Functions ──
static inline void outb(u16 port, u8 val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void uart_send(u8 byte) {
    while (!(inb(UART_LSR) & UART_LSR_THRE));
    outb(UART_DATA, byte);
}

static u8 uart_recv(void) {
    while (!(inb(UART_LSR) & UART_LSR_DR));
    return inb(UART_DATA);
}

// ── CRC16 for Modbus RTU ──
static u16 modbus_crc16(u8 *buffer, u32 length) {
    u16 crc = 0xFFFF;

    for (u32 i = 0; i < length; i++) {
        crc ^= buffer[i];
        for (u8 j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

// ────────────────────────────────────────────────────────────────────────
// Modbus Driver Functions
// ────────────────────────────────────────────────────────────────────────

/**
 * Initialize Modbus RTU (9600 baud, 8N1)
 */
void modbus_init(void) {
    // Disable interrupts
    outb(UART_IER, 0x00);

    // Set baud rate: 9600 (divisor = 115200 / 9600 = 12)
    outb(UART_BASE + 3, 0x80);  // Enable DLAB
    outb(UART_BASE + 0, 12);    // Divisor low
    outb(UART_BASE + 1, 0);     // Divisor high

    // 8 bits, no parity, 1 stop bit
    outb(UART_BASE + 3, 0x03);

    // Enable FIFO
    outb(UART_BASE + 2, 0xC7);

    // DTR + RTS
    outb(UART_BASE + 4, 0x03);
}

/**
 * Read holding register (function code 0x03)
 */
u16 modbus_read_holding_register(u8 slave_addr, u16 reg_addr) {
    u8 request[8];
    u8 response[7];

    // Build request
    request[0] = slave_addr;
    request[1] = MODBUS_READ_HOLDING;
    request[2] = (reg_addr >> 8) & 0xFF;
    request[3] = reg_addr & 0xFF;
    request[4] = 0x00;  // Read 1 register (high byte)
    request[5] = 0x01;  // Read 1 register (low byte)

    // Calculate CRC
    u16 crc = modbus_crc16(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;

    // Send request
    for (u32 i = 0; i < 8; i++) {
        uart_send(request[i]);
    }

    // Receive response
    for (u32 i = 0; i < 7; i++) {
        response[i] = uart_recv();
    }

    // Verify CRC
    u16 recv_crc = response[5] | (response[6] << 8);
    u16 calc_crc = modbus_crc16(response, 5);

    if (recv_crc != calc_crc) {
        return 0xFFFF; // CRC error
    }

    // Extract register value
    return (response[3] << 8) | response[4];
}

/**
 * Write single register (function code 0x06)
 */
bool modbus_write_register(u8 slave_addr, u16 reg_addr, u16 value) {
    u8 request[8];

    request[0] = slave_addr;
    request[1] = MODBUS_WRITE_REGISTER;
    request[2] = (reg_addr >> 8) & 0xFF;
    request[3] = reg_addr & 0xFF;
    request[4] = (value >> 8) & 0xFF;
    request[5] = value & 0xFF;

    u16 crc = modbus_crc16(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;

    for (u32 i = 0; i < 8; i++) {
        uart_send(request[i]);
    }

    // Wait for echo response
    u8 response[8];
    for (u32 i = 0; i < 8; i++) {
        response[i] = uart_recv();
    }

    // Verify echo matches request
    for (u32 i = 0; i < 8; i++) {
        if (request[i] != response[i]) {
            return false;
        }
    }

    return true;
}
