/*#include "../include/printf_color.h"
#include <stdio.h>
#include <stdarg.h>

void vprintf_color(const char *color, const char *format, va_list args) {
    printf("%s", color);
    vprintf(format, args);
    printf("\033[0m\n");
}

void printf_color(const char *color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color(color, format, args);
    va_end(args);
}

void printf_red(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color("\033[31m", format, args);
    va_end(args);
}

void printf_blue(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color("\033[34m", format, args);
    va_end(args);
}

void printf_yellow(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color("\033[93m", format, args);
    va_end(args);
}

void printf_orange(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color("\033[38;2;255;165;0m", format, args);
    va_end(args);
}

void printf_green(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color("\033[32m", format, args);
    va_end(args);
}

void printf_purple(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color("\033[35m", format, args);
    va_end(args);
}*/