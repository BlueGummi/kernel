#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

struct flanterm_context *ft_ctx;
#define WHITE_TXT 0x07

void k_clear_screen();
unsigned int k_printf(const char *format, ...);


// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

#include "gdt.h"
#include "idt.h"

void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    ft_ctx = flanterm_fb_init(
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
        0
    );
    k_printf("I like to make walks in the morning, ja? with my legs\n");
    k_printf("Integer: %d, String: %s, Char: %c, Hex: %x\n", 42, "test", 'A', 0xDEADBEEF);
    gdt_install();
    k_printf("i have installed the GDT :shocked:\n");

    init_interrupts();

    enable_interrupts();

    k_printf("i have installed the IDT :omg:\n");
    while (1) {}
}

unsigned int k_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
            case 'd': {
                int num = va_arg(args, int);
                char buffer[32];
                int n = 0;
                if (num < 0) {
                    flanterm_write(ft_ctx, "-", 1);
                    num = -num;
                }
                do {
                    buffer[n++] = '0' + (num % 10);
                    num /= 10;
                } while (num > 0);
                flanterm_write(ft_ctx, buffer, 32);
                break;
            }
            case 's': {
                char *str = va_arg(args, char *);
                flanterm_write(ft_ctx, str, sizeof(str));
                break;
            }
            case 'c': {
                char c = va_arg(args, int);
                char str[2];
                str[0] = c;
                str[1] = '\0';
                flanterm_write(ft_ctx, str, 2);
                break;
            }
            case 'x': {
                unsigned int num = va_arg(args, unsigned int);
                char buffer[32];
                int n = 0;
                do {
                    int rem = num % 16;
                    buffer[n++] = (rem < 10) ? ('0' + rem) : ('a' + (rem - 10));
                    num /= 16;
                } while (num > 0);
                flanterm_write(ft_ctx, buffer, 32);
                break;
            }
            case '%': {
                flanterm_write(ft_ctx, "%", 1);
                break;
            }
            default: {
                flanterm_write(ft_ctx, "%", 1);
                flanterm_write(ft_ctx, format, sizeof(format));
                break;
            }
            }
        } else if (*format == '\n') {
            flanterm_write(ft_ctx, "\n", sizeof("\n"));
        } else {
            char str[2];
            str[0] = *format;
            str[1] = '\0';
            flanterm_write(ft_ctx, str, 2);
        }
        format++;
    }

    va_end(args);
    return 1;
}
