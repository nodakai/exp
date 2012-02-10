#include <iostream>
#include "handleFuncs.h"

using namespace std;

int main(int argc, char *argv[]) {
    const int N = argc > 1 ? atoi(argv[1]) : 1000 * 1000;
    const clock_t t0 = clock();
    for (int i = 0; i < N; ++i) {
        a::handle hdl = argv[0];
        a::foo(hdl);

        // a::handle1 hdl1 = argv[0];
        // hdl1 = hdl;
    }
    const clock_t t1 = clock();
    for (int i = 0; i < N; ++i) {
        b::handle<b::x> hdl = argv[0];
        b::foo(hdl);

        // b::handle<b::y> hdl1 = argv[0];
        // hdl1 = hdl;

        // b::handle<a::handle> hdl1 = argv[0];
        // b::handle<a::handle1> hdl2 = argv[0];
        // hdl1 = hdl2;
    }
    const clock_t t2 = clock();

    cout << (t1 - t0) / double(CLOCKS_PER_SEC) << endl;
    cout << (t2 - t1) / double(CLOCKS_PER_SEC) << endl;

    cout << a::bar() << endl;
    cout << b::bar() << endl;
}
