#pragma once

#include <cstdlib>

namespace a {
typedef void *handle;
typedef void *handle1;

void foo(handle hdl);
size_t bar();
}

namespace b {
template <typename TTag>
class handle {
private:
    typedef TTag not_used;
    void *m_p;

public:
    explicit handle(void *p) : m_p(p) { }

    template<typename U>
    const U *ptr() const { return reinterpret_cast<U*>(m_p); }

    template<typename U>
    U *ptr() { return reinterpret_cast<U*>(m_p); }
};

struct x { };
struct y { };

void foo(handle<x> hdl);
size_t bar();
}
