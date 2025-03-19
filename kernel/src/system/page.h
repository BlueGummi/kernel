#include <stdint.h>
#include <stddef.h>
#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITE      (1 << 1)
#define PAGE_USER       (1 << 2)
#define PAGE_HUGE       (1 << 7)
void map_page(uint64_t virt, uint64_t phys, uint64_t flags);
void init_paging(); 
