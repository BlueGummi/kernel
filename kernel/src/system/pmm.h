#include <stddef.h>
void init_physical_allocator();
void *alloc_page();
void *allocate_page();
void free_page(void *addr);
void *alloc_pages(size_t count);
void free_pages(void *addr, size_t count);
