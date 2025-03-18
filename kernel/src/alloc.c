#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <system/memfuncs.h>
#include <system/printf.h>

#define PAGE_SIZE 4096
#define BITMAP_SIZE (MEMORY_SIZE / PAGE_SIZE / 8)

#define MEMORY_SIZE 0x100000000
#define KERNEL_START 0x100000

static uint8_t bitmap[BITMAP_SIZE];

static void set_bit(size_t index) {
    bitmap[index / 8] |= (1 << (index % 8));
}

static void clear_bit(size_t index) {
    bitmap[index / 8] &= ~(1 << (index % 8));
}

static bool test_bit(size_t index) {
    return (bitmap[index / 8] & (1 << (index % 8))) != 0;
}

void init_physical_allocator() {
    memset(bitmap, 0xFF, BITMAP_SIZE);

    for (size_t i = 0; i < KERNEL_START / PAGE_SIZE; i++) {
        clear_bit(i);
    }
}

void *alloc_page() {
    for (size_t i = 0; i < BITMAP_SIZE * 8; i++) {
        if (test_bit(i)) {
            clear_bit(i);
            return (void *) (i * PAGE_SIZE);
        }
    }
    return NULL;
}

void free_page(void *addr) {
    size_t index = (size_t) addr / PAGE_SIZE;
    set_bit(index);
}

void *alloc_pages(size_t count) {
    size_t consecutive = 0;
    for (size_t i = 0; i < BITMAP_SIZE * 8; i++) {
        if (test_bit(i)) {
            consecutive++;
            if (consecutive == count) {
                for (size_t j = i - count + 1; j <= i; j++) {
                    clear_bit(j);
                }
                return (void *) ((i - count + 1) * PAGE_SIZE);
            }
        } else {
            consecutive = 0;
        }
    }
    return NULL;
}

void free_pages(void *addr, size_t count) {
    size_t index = (size_t) addr / PAGE_SIZE;
    for (size_t i = index; i < index + count; i++) {
        set_bit(i);
    }
}

void print_memory_status() {
    size_t total_pages = BITMAP_SIZE * 8;
    size_t free_pages = 0;
    size_t allocated_pages = 0;

    for (size_t i = 0; i < total_pages; i++) {
        if (test_bit(i)) {
            free_pages++;
        } else {
            allocated_pages++;
        }
    }

    k_printf("Memory Status:\n");
    k_printf("  Total Pages: %zu\n", total_pages);
    k_printf("  Free Pages: %zu\n", free_pages);
    k_printf("  Allocated Pages: %zu\n", allocated_pages);
    k_printf("  Memory Usage: %d%%\n", (allocated_pages * 100) / total_pages);
    k_printf("\nDetailed Allocation Map:\n");
    for (size_t i = 0; i < total_pages; i++) {
        if (i % 64 == 0) {
            k_printf("\n");
        }
        k_printf("%c", test_bit(i) ? '.' : 'X');
    }
    k_printf("\n");
}
