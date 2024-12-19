#ifndef DEBUG_PANIC_H
#define DEBUG_PANIC_H


[[noreturn]] inline extern void panic() {
    *(volatile int *)nullptr = 0xf;
    exit(-1);
}
#endif //DEBUG_PANIC_H
