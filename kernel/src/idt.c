#include <stdint.h>
#include "io.h"
#include "printf.h"

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
#define YELL do { k_printf("someone hit a button\n"); while(1){}} while (0)

void remap_pic() {
    // ICW1: Initialize
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2: Remap offsets
    outb(0x21, 0x20); // Master PIC starts at 0x20 (32)
    outb(0xA1, 0x28); // Slave PIC starts at 0x28 (40)

    // ICW3: Cascade setup
    outb(0x21, 0x04); // Master has slave at IRQ2
    outb(0xA1, 0x02); // Slave identity

    // ICW4: Environment info
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask interrupts (OCW1)
    outb(0x21, 0xFD); // Enable IRQ1 (keyboard) on master
    outb(0xA1, 0xFF); // Disable all slave interrupts
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

__attribute__((interrupt)) void keyboard_handler(struct interrupt_frame *frame) {
    YELL;
    uint8_t keycode = inb(0x60);
    k_printf("Keycode: 0x%x\n", keycode);

    outb(0x20, 0x20);
}

void idt_install() {
    idtp.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idtp.base = (uint64_t)&idt;
    asm volatile("lidt %0" : : "m"(idtp));
}

void init_interrupts() {
    remap_pic(); 

    // Set up keyboard interrupt (IRQ1 -> vector 33)
    idt_set_gate(33, (uint64_t)keyboard_handler, 0x08, 0x8E);

    idt_install();
    asm volatile("sti");
}
