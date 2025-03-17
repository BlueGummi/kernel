#include <stdint.h>
#include <stddef.h>

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
void idt_install();
void irq_handler();
void init_interrupts();
void enable_interrupts();

struct idt_entry {
    uint16_t base_low;      // The lower 16 bits of the address to jump to
    uint16_t selector;      // Kernel segment selector
    uint8_t  ist;           // Interrupt Stack Table offset
    uint8_t  flags;         // Type and attributes
    uint16_t base_mid;      // The middle 16 bits of the address to jump to
    uint32_t base_high;     // The high 32 bits of the address to jump to
    uint32_t reserved;      // Reserved
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#define IDT_ENTRIES 256
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}
void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint64_t)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);

    asm volatile("lidt %0" : : "m"(idtp));
}

void irq_handler() {
    k_printf("Interrupt occurred!\n");

    outb(0x20, 0x20); // Send EOI to master PIC
}

void init_interrupts() {
    idt_set_gate(32, (uint64_t)irq_handler, 0x08, 0x8E);

    idt_install();
}

inline void enable_interrupts() {
    asm volatile("sti");
}
