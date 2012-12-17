#include <iostream>
#include <climits>
#include <stdint.h>

using namespace std;

enum class E : uint32_t /* uint64_t */ {
    A = 0,
    B = 'b',
    C = 'c',
    D = UINT32_MAX
};

int main() {
    E e = E::A;
    cout << static_cast<uint32_t>(e) << endl;
    e = E::B;
    cout << static_cast<char>(e) << endl;
    e = E::D;
    cout << static_cast<uint32_t>(e) << endl;
    cout << static_cast<int>(e) << endl;
    cout << sizeof e << endl;
    // e = 0;
}
