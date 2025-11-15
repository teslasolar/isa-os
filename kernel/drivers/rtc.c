/**
 * ════════════════════════════════════════════════════════════════════════
 * Real-Time Clock (rtc.c)
 * CMOS RTC driver for timekeeping
 * ════════════════════════════════════════════════════════════════════════
 */

#include "../include/isa_types.h"
#include "../include/module.h"

#define RTC_ADDR 0x70
#define RTC_DATA 0x71

// RTC registers
#define RTC_SECONDS     0x00
#define RTC_MINUTES     0x02
#define RTC_HOURS       0x04
#define RTC_DAY         0x07
#define RTC_MONTH       0x08
#define RTC_YEAR        0x09
#define RTC_STATUS_A    0x0A
#define RTC_STATUS_B    0x0B

typedef struct {
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u16 year;
} datetime_t;

static datetime_t current_time;

// ────────────────────────────────────────────────────────────────────────
// Port I/O
// ────────────────────────────────────────────────────────────────────────

static inline void outb(u16 port, u8 val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// ────────────────────────────────────────────────────────────────────────
// RTC Functions
// ────────────────────────────────────────────────────────────────────────

static u8 rtc_read(u8 reg) {
    outb(RTC_ADDR, reg);
    return inb(RTC_DATA);
}

static void rtc_write(u8 reg, u8 value) {
    outb(RTC_ADDR, reg);
    outb(RTC_DATA, value);
}

static u8 bcd_to_binary(u8 bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static u8 binary_to_bcd(u8 binary) {
    return ((binary / 10) << 4) | (binary % 10);
}

static bool is_updating(void) {
    outb(RTC_ADDR, RTC_STATUS_A);
    return inb(RTC_DATA) & 0x80;
}

static void rtc_read_time(datetime_t *dt) {
    // Wait for update to complete
    while (is_updating());

    dt->second = bcd_to_binary(rtc_read(RTC_SECONDS));
    dt->minute = bcd_to_binary(rtc_read(RTC_MINUTES));
    dt->hour = bcd_to_binary(rtc_read(RTC_HOURS));
    dt->day = bcd_to_binary(rtc_read(RTC_DAY));
    dt->month = bcd_to_binary(rtc_read(RTC_MONTH));
    dt->year = bcd_to_binary(rtc_read(RTC_YEAR)) + 2000;
}

static i32 rtc_init(void) {
    extern void kprint(const char *);
    extern void kprinthex(u32);

    rtc_read_time(&current_time);

    kprint("[RTC] Current time: ");
    kprinthex(current_time.year);
    kprint("-");
    kprinthex(current_time.month);
    kprint("-");
    kprinthex(current_time.day);
    kprint(" ");
    kprinthex(current_time.hour);
    kprint(":");
    kprinthex(current_time.minute);
    kprint(":");
    kprinthex(current_time.second);
    kprint("\n");

    return 0;
}

static void rtc_start(void) {
    // Enable RTC periodic interrupt (optional)
}

static void rtc_stop(void) {
    // Disable interrupts
}

static void rtc_cleanup(void) {}

/**
 * Get current time
 */
void rtc_get_time(datetime_t *dt) {
    rtc_read_time(dt);
}

/**
 * Get Unix timestamp (seconds since 1970)
 */
u64 rtc_get_timestamp(void) {
    datetime_t dt;
    rtc_read_time(&dt);

    // Simplified timestamp calculation
    // Days since epoch
    u32 days = (dt.year - 1970) * 365 + dt.day;

    // Add months
    u32 month_days[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    if (dt.month <= 12) {
        days += month_days[dt.month - 1];
    }

    // Calculate seconds
    u64 seconds = days * 86400ULL;
    seconds += dt.hour * 3600;
    seconds += dt.minute * 60;
    seconds += dt.second;

    return seconds;
}

// ── Module Registration ──
MODULE_REGISTER(rtc, "Real-Time Clock", MODULE_TYPE_DRIVER, ISA_L0_FIELD);
