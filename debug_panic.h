#ifndef DEBUG_PANIC_H
#define DEBUG_PANIC_H

[[noreturn]] __forceinline extern void panic() {
    *(int *)nullptr = 0xf;
    exit(-1);
}
#endif //DEBUG_PANIC_H