#include <flanterm/backends/fb.h>
#include <flanterm/flanterm.h>
#include <limine.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <system/dsdt.h>
#include <system/fadt.h>
#include <system/gdt.h>
#include <system/idt.h>
#include <system/io.h>
#include <system/memfuncs.h>
#include <system/page.h>
#include <system/pmm.h>
#include <system/printf.h>
#include <system/rsdp.h>
#include <system/shutdown.h>
#include <system/smap.h>

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
    asm("hlt");
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
    struct Rsdp rsdp = make_rsdp((void *) ((0x3000) + (rsdp_request.response->address & 0xFFF)));
    paging_map_cr3((void *) (get_cr3() + response->offset), (uint64_t) rsdp.rsdt_address, 0x5000, PAGING_X86_64_PRESENT);

    struct ACPI_SDTHeader *rsdt = (struct ACPI_SDTHeader *) (0x5000 + (rsdp.rsdt_address & 0xFFF));
    uint32_t *entries = (uint32_t *) ((uintptr_t) rsdt + sizeof(struct ACPI_SDTHeader));
    size_t entry_count = (rsdt->length - sizeof(struct ACPI_SDTHeader)) / 4;

    struct FADT *fadt = NULL;
    for (size_t i = 0; i < entry_count; i++) {
        paging_map_cr3((void *) (get_cr3() + response->offset), (uint64_t) entries[i], 0x6000, PAGING_X86_64_PRESENT);
        struct ACPI_SDTHeader *table = (struct ACPI_SDTHeader *) (0x6000 + (entries[i] & 0xFFF));

        if (memcmp(table->sig, "FACP", 4) == 0) {
            fadt = (struct FADT *) table;
            break;
        }
    }

    paging_map_cr3((void *) (get_cr3() + response->offset), (uint64_t) fadt->DSDT, 0x7000, PAGING_X86_64_PRESENT);
    struct ACPI_SDTHeader *dsdt = (struct ACPI_SDTHeader *) (0x7000 + (fadt->DSDT & 0xFFF));

    uint16_t SLP_TYP = find_s5_in_dsdt((uint8_t *) dsdt, dsdt->length);

    if (SLP_TYP != 0xFFFF) {
        k_shutdown_init(fadt->PM1a_CNT_BLK, fadt->PM1b_CNT_BLK, SLP_TYP);
    }

    while (1) {
        asm("hlt");
        k_shutdown();
    }
}
