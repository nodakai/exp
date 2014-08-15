#include <iostream>
#include <vector>

using namespace std;

#include <stdint.h>

#include <time.h>

typedef vector<int> V;

static int N = 1000 * 1000;

static int64_t nanoTime() {
    ::timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);
    return int64_t(ts.tv_sec) * 1000 * 1000 * 1000 + ts.tv_nsec;
}

static int64_t timeIt(size_t n) {
    for (int i = 0; i < 100; ++i) {
        V vec(n);
    }
    const int64_t t0 = nanoTime();
    for (int i = 0; i < N; ++i) {
        V vec(n);
    }
    const int64_t t1 = nanoTime();
    return t1 - t0;
}

int main() {
    const int avg100 = timeIt(100) / N;
    const int avg1000 = timeIt(1000) / N;
    const int avg10000 = timeIt(10000) / N;
    const int avg100000 = timeIt(100000) / N;

    cout << "n==100 => " << avg100 << endl;
    cout << "n==1000 => " << avg1000 << endl;
    cout << "n==10000 => " << avg10000 << endl;
    cout << "n==100000 => " << avg100000 << endl;
}
