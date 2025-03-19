#include <stdint.h>
#include <system/pmm.h>
#include <system/printf.h>
#include <stddef.h>

#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITE      (1 << 1)
#define PAGE_USER       (1 << 2)
#define PAGE_HUGE       (1 << 7)
extern void *alloc_page(void);
#define PHYS_MAP_BASE   0xffff800000000000

#define KERNEL_VIRT_BASE 0xffff800000000000

typedef uint64_t pt_entry;

pt_entry *pml4;

#define TO_VIRT(paddr) ((uint64_t)(paddr) + PHYS_MAP_BASE)

#define TO_PHYS(vaddr) ((uint64_t)(vaddr) - PHYS_MAP_BASE)

void map_page(uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;
    pt_entry *pml4_entry = &pml4[pml4_idx];
    if (!(*pml4_entry & PAGE_PRESENT)) {
        uint64_t* pdpt_phys = alloc_page();
        *pml4_entry = (uint64_t) pdpt_phys | PAGE_PRESENT | PAGE_WRITE;
    }
    pt_entry *pdpt = (pt_entry*)TO_VIRT(*pml4_entry & ~0xFFF);
    pdpt = alloc_page();
    pt_entry *pdpt_entry = &pdpt[pdpt_idx];
    if (!(*pdpt_entry & PAGE_PRESENT)) {
        uint64_t* pd_phys = alloc_page();
        *pdpt_entry = (uint64_t) pd_phys | PAGE_PRESENT | PAGE_WRITE;
    }
    pt_entry *pd = (pt_entry*)TO_VIRT(*pdpt_entry & ~0xFFF);
    pd = alloc_page();
    pt_entry *pd_entry = &pd[pd_idx];
    if (!(*pd_entry & PAGE_PRESENT)) {
        uint64_t* pt_phys = alloc_page();
        *pd_entry = (uint64_t) pt_phys | PAGE_PRESENT | PAGE_WRITE;
    }
    pt_entry *pt = (pt_entry*)TO_VIRT(*pd_entry & ~0xFFF);
    pt = alloc_page();
    pt_entry *pt_entry = &pt[pt_idx];
    *pt_entry = phys | flags;

    asm volatile("invlpg (%0)" : : "r" (virt) : "memory");
}


void init_paging(uint64_t offset) {
    pml4 = alloc_page();
    for (uint64_t i = 0; i < 512; i++) {
        map_page(i * 0x200000, i * 0x200000, PAGE_PRESENT | PAGE_WRITE | PAGE_HUGE);
        
        map_page(KERNEL_VIRT_BASE + i * 0x200000, i * 0x200000, 
                PAGE_PRESENT | PAGE_WRITE | PAGE_HUGE);
    }

    for (uint64_t pt = 0x1000; pt < 0x5000; pt += 0x1000) {
        map_page(TO_VIRT(pt), pt, PAGE_PRESENT | PAGE_WRITE);
    }
    k_printf("I sit at 0x%zx\n", pml4);
    asm volatile(
        "mov %0, %%cr3\n"
        : : "r" ((uint64_t)pml4 - offset) );
    k_printf("got\n");
}
