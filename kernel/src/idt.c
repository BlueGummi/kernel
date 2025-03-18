#include <stdint.h>
#include <system/dbg.h>
#include <system/io.h>
#include <system/kb.h>
#include <system/printf.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#define IDT_ENTRIES 256
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;
#define YELL                                              \
    do {                                                  \
        k_printf("========== MANUAL HALT! ==========\n"); \
        while (1) {                                       \
            asm("hlt");                                   \
        }                                                 \
    } while (0)

void remap_pic() {
    outb(0x20, 0x11); // Start initialization of master PIC
    outb(0xA0, 0x11); // Start initialization of slave PIC

    outb(0x21, 0x20); // Set master PIC offset to 0x20
    outb(0xA1, 0x28); // Set slave PIC offset to 0x28

    outb(0x21, 0x04); // Master PIC IRQ2 connected to slave
    outb(0xA1, 0x02); // Slave PIC connected to IRQ2 on master

    outb(0x21, 0x01); // Set 8086 mode for master PIC
    outb(0xA1, 0x01); // Set 8086 mode for slave PIC

    outb(0x21, 0xFD); // Enable IRQ1 (keyboard) on master PIC
    outb(0xA1, 0xFF); // Disable all interrupts on slave PIC
}

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}
struct interrupt_frame {
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
};

void idt_install() {
    idtp.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idtp.base = (uint64_t) &idt;
    asm volatile("lidt %0" : : "m"(idtp));
}

void init_interrupts() {
    remap_pic();

    idt_set_gate(33, (uint64_t) keyboard_handler, 0x08, 0x8E);

    idt_install();

    asm volatile("sti");
}
