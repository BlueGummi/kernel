static void set_bit(size_t index);
static void clear_bit(size_t index);
static bool test_bit(size_t index); 
void init_physical_allocator(); 
void* alloc_page(); 
void free_page(void* addr); 
void* alloc_pages(size_t count); 
void free_pages(void* addr, size_t count); 
