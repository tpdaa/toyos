#include "printf.h"
#include "sbi.h"
#include <stdarg.h>

static void console_putc(int ch)
{
    sbi_console_putchar(ch);
}

static void printstr(const char *s)
{
    if (s == 0) {
        s = "(null)";
    }

    while (*s) {
        console_putc(*s);
        s++;
    }
}

static void reverse(char *buf, int len)
{
    int i = 0;
    int j = len - 1;
    while (i < j) {
        char tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
        i++;
        j--;
    }
}

static void itoa_unsigned(unsigned long x, int base, char *buf)
{
    int i = 0;

    if (x == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    while (x > 0) {
        int digit = x % base;
        if (digit < 10) {
            buf[i++] = '0' + digit;
        } else {
            buf[i++] = 'a' + (digit - 10);
        }
        x /= base;
    }

    buf[i] = '\0';
    reverse(buf, i);
}

static void printint(long x, int base, int sign)
{
    char buf[32];
    unsigned long ux;

    if (sign && x < 0) {
        console_putc('-');
        ux = (unsigned long)(-x);
    } else {
        ux = (unsigned long)x;
    }

    itoa_unsigned(ux, base, buf);
    printstr(buf);
}

static void printptr(const void *p)
{
    unsigned long x = (unsigned long)p;
    char buf[32];

    printstr("0x");
    itoa_unsigned(x, 16, buf);
    printstr(buf);
}

void printf(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    for (;;) {
        switch (*fmt) {
        case '\0':
            va_end(list);
            return;

        case '%':
            fmt++;
            switch (*fmt) {
            case '\0':
                va_end(list);
                return;

            case '%':
                console_putc('%');
                break;

            case 'c':
                console_putc(va_arg(list, int));
                break;

            case 'd':
                printint(va_arg(list, int), 10, 1);
                break;

            case 'x':
                printint(va_arg(list, int), 16, 0);
                break;

            case 's':
                printstr(va_arg(list, const char *));
                break;

            case 'p':
                printptr(va_arg(list, const void *));
                break;

            case 'l':
                fmt++;
                switch (*fmt) {
                case 'd':
                    printint(va_arg(list, long), 10, 1);
                    break;
                case 'x':
                    printint(va_arg(list, unsigned long), 16, 0);
                    break;
                default:
                    console_putc('%');
                    console_putc('l');
                    console_putc(*fmt);
                    break;
                }
                break;

            default:
                console_putc('%');
                console_putc(*fmt);
                break;
            }
            break;

        default:
            console_putc(*fmt);
            break;
        }

        fmt++;
    }
}