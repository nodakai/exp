#include "handleFuncs.h"

namespace a {
static size_t cnt = 0;
void foo(handle hdl) {
    cnt += reinterpret_cast<size_t>(hdl);
}

size_t bar() { return cnt; }
}

namespace b {
static size_t cnt = 0;
void foo(handle<x> hdl) {
    cnt += reinterpret_cast<size_t>(hdl.ptr<void*>());
}

size_t bar() { return cnt; }
}
