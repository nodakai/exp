#include <iostream>
#include <stdexcept>

using std::string;
using std::cout;
using std::endl;

struct TestNullptr {
    static void foo(int x) { cout << __func__ << ": " << x << endl; }
    static void foo(char *x) { cout << __func__ << ": " << static_cast<void*>(x) << endl; }
};

void testNullptr() {
    TestNullptr::foo(0);
    // TestNullptr::foo(NULL);
    TestNullptr::foo(nullptr);
}

////////////////////////////////////////////////////////////////////////////////

struct ArrayInitTemp {
    static int x;

    ArrayInitTemp() {
        if (0 < x++)
            throw std::logic_error("");
    }

    ArrayInitTemp() {
    }
};
int ArrayInitTemp::x = 0;

int main() {
    testNullptr();


    struct ArrayElement {
        ArrayElement() {
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
}
