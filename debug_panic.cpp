#include "debug_panic.h"
#include <cpptrace/cpptrace.hpp>


//--------------------------------------------------------------------------------------------------
// panic_impl
//--------------------------------------------------------------------------------------------------
[[noreturn]] void panic_impl(const char* s) noexcept {
    std::fputs(s, stderr);
    __asm__("nop");
    cpptrace::generate_trace().print();
    __builtin_trap();
    std::abort();
}
