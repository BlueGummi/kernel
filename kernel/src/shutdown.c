#include <stdint.h>
#include <stddef.h>

static inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %1, %0" :: "dN" (port), "a" (value));
}

void acpi_shutdown(uint16_t pm1a_control, uint16_t pm1b_control, uint16_t slp_typ) {
    uint16_t SLP_EN = 1 << 13;
    uint16_t sleep_command = slp_typ | SLP_EN;

    outw(pm1a_control, sleep_command);

    if (pm1b_control) {
        outw(pm1b_control, sleep_command);
    }
}

