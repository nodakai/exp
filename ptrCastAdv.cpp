#include <iostream>
#include <iomanip>

#include <stdint.h>

using namespace std;

struct Foo {
    uint8_t
        x, y, z,
        p, q, r,
        u, v, w,
        a, b, c;
};

#define PRT(p, name) static_cast<int>(p->name)
#define PRT_ALL(p) \
    cout << p << " : " << PRT(p, x) << " " << PRT(p, y) << " " << PRT(p, z) << " ... " << \
        PRT(p, a) << " " << PRT(p, b) << " " << PRT(p, c) << endl

int main()
{
    Foo foos[3] = {
        { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 },
        { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112 },
        { 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212 }
    };
    Foo *p = foos;
    PRT_ALL(p);
    p = reinterpret_cast<Foo *>(reinterpret_cast<char *>(p) + 1);
    // ++reinterpret_cast<char *>(p);
    // ++(char *)p;
    // (char *)p += 1;
    PRT_ALL(p);
}
