#ifndef DEBUG_PANIC_H
#define DEBUG_PANIC_H


[[noreturn]] inline extern void panic() {
    *(volatile int *)nullptr = 0xf;
    exit(-1);
}
inline extern void vkc(VkResult result,const char* fail_text="") {
    if (result != VK_SUCCESS) {
        panic();
    }
}
#endif //DEBUG_PANIC_H
