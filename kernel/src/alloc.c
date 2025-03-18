#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "system/memfuncs.h"
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

void* alloc_page() {
    for (size_t i = 0; i < BITMAP_SIZE * 8; i++) {
        if (test_bit(i)) {
            clear_bit(i); 
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void free_page(void* addr) {
    size_t index = (size_t)addr / PAGE_SIZE;
    set_bit(index);
}

void* alloc_pages(size_t count) {
    size_t consecutive = 0;
    for (size_t i = 0; i < BITMAP_SIZE * 8; i++) {
        if (test_bit(i)) {
            consecutive++;
            if (consecutive == count) {
                for (size_t j = i - count + 1; j <= i; j++) {
                    clear_bit(j);
                }
                return (void*)((i - count + 1) * PAGE_SIZE);
            }
        } else {
            consecutive = 0;
        }
    }
    return NULL; 
}

void free_pages(void* addr, size_t count) {
    size_t index = (size_t)addr / PAGE_SIZE;
    for (size_t i = index; i < index + count; i++) {
        set_bit(i);
    }
}
