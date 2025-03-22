#include <flanterm/backends/fb.h>
#include <flanterm/flanterm.h>
#include <limine.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <system/gdt.h>
#include <system/idt.h>
#include <system/io.h>
#include <system/page.h>
#include <system/pmm.h>
#include <system/printf.h>
#include <system/smap.h>

struct __attribute__((packed)) Rsdp {
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;  // ACPI 1.0
};


extern struct Rsdp make_rsdp(void *rsdp_addr);

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_executable_address_request addr_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0};

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    for (;;) {
#if defined(__x86_64__)
        asm("hlt");
#elif defined(__aarch64__) || defined(__riscv)
        asm("wfi");
#elif defined(__loongarch64)
        asm("idle 0");
#endif
    }
}

void kmain(void) {
    struct limine_hhdm_response *response = hhdm_request.response;
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct flanterm_context *ft_ctx = flanterm_fb_init(
        NULL,
        NULL,
        framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch,
        framebuffer->red_mask_size, framebuffer->red_mask_shift,
        framebuffer->green_mask_size, framebuffer->green_mask_shift,
        framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
        NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, 0, 0, 1,
        0, 0,
        0);
    k_printf_init(ft_ctx);

    enable_smap_smep_umip();
    gdt_install();
    init_interrupts();
    init_physical_allocator(response->offset, memmap_request);
    init_paging(response->offset);
    int *p = pmm_alloc_page();

    *p = 42;

    paging_map_cr3((void *) (get_cr3() + response->offset), (uint64_t) rsdp_request.response->address, 0x3000, PAGING_X86_64_PRESENT);

    struct Rsdp rsdp = make_rsdp((void*) (0x3310));
    k_printf("Signature: %s\n", rsdp.signature);
    k_printf("OEM ID: %s\n", rsdp.oem_id);
    k_printf("Revision: %hhu\n", rsdp.revision);
    k_printf("RSDT Address: 0x%x\n", rsdp.rsdt_address);


    k_printf("done\n");
    while (1) {
        asm("hlt");
    }
}
