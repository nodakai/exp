#include <tbb/atomic.h>
#include <tbb/compat/thread>

using namespace std;

tbb::atomic<int> x, y;

int foo() {
    int cnt = 0;
    for (;;) {
        int xx = x;
        int yy = y;
        if (xx > 1000) break;
        cnt += xx + yy;
    }
    return cnt;
}

void bar() {
    int c = 0;
    while (c++ < 1000) {
        y = c;
        x = c;
    }
}

int main() {
    x = 0;
    y = 0;
    thread t0(foo);
    thread t1(bar);

    t0.join();
    t1.join();
}
