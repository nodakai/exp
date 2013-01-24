#include <atomic>

std::atomic_int x;
int y;
char buf[1024];
void g(int x);

void a0()
{
    while (0 != x.load(std::memory_order_acquire))
        ;
    buf[0] = 0;
    x.store(1, std::memory_order_release);
}

void a1()
{
    while (1 != x.load(std::memory_order_acquire))
        ;
    g( buf[0] );
    x.store(1, std::memory_order_release);
}

void b0()
{
    while (0 != x.load(std::memory_order_seq_cst))
        ;
    buf[0] = 0;
    x.store(1, std::memory_order_seq_cst);
}

void b1()
{
    while (1 != x.load(std::memory_order_seq_cst))
        ;
    g( buf[0] );
    x.store(1, std::memory_order_seq_cst);
}

void c0()
{
    while (0 != x.load(std::memory_order_relaxed))
        ;
    buf[0] = 0;
    x.store(1, std::memory_order_relaxed);
}

void c1()
{
    while (1 != x.load(std::memory_order_relaxed))
        ;
    g( buf[0] );
    x.store(1, std::memory_order_relaxed);
}

void d0()
{
    while (0 != y)
        ;
    buf[0] = 0;
    y = 1;
}

void d1()
{
    while (1 != y)
        ;
    g( buf[0] );
    y = 1;
}
