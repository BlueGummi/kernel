#include <stdarg.h>
#include <stddef.h>
#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>

struct flanterm_context *ft_ctx;

void k_printf_init(struct flanterm_context *f) {
    ft_ctx = f;
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
