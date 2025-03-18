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
                while (n > 0) {
                    flanterm_write(ft_ctx, &buffer[--n], 1);
                }
                break;
            }
            case 'u': {
                unsigned int num = va_arg(args, unsigned int);
                char buffer[32];
                int n = 0;
                do {
                    buffer[n++] = '0' + (num % 10);
                    num /= 10;
                } while (num > 0);
                while (n > 0) {
                    flanterm_write(ft_ctx, &buffer[--n], 1);
                }
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
                while (n > 0) {
                    flanterm_write(ft_ctx, &buffer[--n], 1);
                }
                break;
            }
            case 's': {
                char *str = va_arg(args, char *);
                size_t len = 0;
                while (str[len] != '\0') len++;
                flanterm_write(ft_ctx, str, len);
                break;
            }
            case 'c': {
                char c = va_arg(args, int);
                flanterm_write(ft_ctx, &c, 1);
                break;
            }
            case '%': {
                flanterm_write(ft_ctx, "%", 1);
                break;
            }
            case 'h': {
                format++;
                if (*format == 'h') {
                    format++;
                    if (*format == 'd') {
                        int8_t num = va_arg(args, int);
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
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'u') {
                        uint8_t num = va_arg(args, unsigned int);
                        char buffer[32];
                        int n = 0;
                        do {
                            buffer[n++] = '0' + (num % 10);
                            num /= 10;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'x') {
                        uint8_t num = va_arg(args, unsigned int);
                        char buffer[32];
                        int n = 0;
                        do {
                            int rem = num % 16;
                            buffer[n++] = (rem < 10) ? ('0' + rem) : ('a' + (rem - 10));
                            num /= 16;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    }
                } else {
                    if (*format == 'd') {
                        int16_t num = va_arg(args, int);
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
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'u') {
                        uint16_t num = va_arg(args, unsigned int);
                        char buffer[32];
                        int n = 0;
                        do {
                            buffer[n++] = '0' + (num % 10);
                            num /= 10;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'x') {
                        uint16_t num = va_arg(args, unsigned int);
                        char buffer[32];
                        int n = 0;
                        do {
                            int rem = num % 16;
                            buffer[n++] = (rem < 10) ? ('0' + rem) : ('a' + (rem - 10));
                            num /= 16;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    }
                }
                break;
            }
            case 'l': {
                format++;
                if (*format == 'l') {
                    format++;
                    if (*format == 'd') {
                        int64_t num = va_arg(args, int64_t);
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
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'u') {
                        uint64_t num = va_arg(args, uint64_t);
                        char buffer[32];
                        int n = 0;
                        do {
                            buffer[n++] = '0' + (num % 10);
                            num /= 10;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'x') {
                        uint64_t num = va_arg(args, uint64_t);
                        char buffer[32];
                        int n = 0;
                        do {
                            int rem = num % 16;
                            buffer[n++] = (rem < 10) ? ('0' + rem) : ('a' + (rem - 10));
                            num /= 16;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    }
                } else {
                    if (*format == 'd') {
                        int32_t num = va_arg(args, int32_t);
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
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'u') {
                        uint32_t num = va_arg(args, uint32_t);
                        char buffer[32];
                        int n = 0;
                        do {
                            buffer[n++] = '0' + (num % 10);
                            num /= 10;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    } else if (*format == 'x') {
                        uint32_t num = va_arg(args, uint32_t);
                        char buffer[32];
                        int n = 0;
                        do {
                            int rem = num % 16;
                            buffer[n++] = (rem < 10) ? ('0' + rem) : ('a' + (rem - 10));
                            num /= 16;
                        } while (num > 0);
                        while (n > 0) {
                            flanterm_write(ft_ctx, &buffer[--n], 1);
                        }
                    }
                }
                break;
            }
            default: {
                flanterm_write(ft_ctx, "%", 1);
                flanterm_write(ft_ctx, format, 1);
                break;
            }
            }
        } else if (*format == '\n') {
            flanterm_write(ft_ctx, "\n", 1);
        } else {
            flanterm_write(ft_ctx, format, 1);
        }
        format++;
    }

    va_end(args);
    return 1;
}
