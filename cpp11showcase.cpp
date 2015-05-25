#include <iostream>

#include <cuchar> // TestChar16

int main() {
    std::cout << __func__ << std::endl;

    struct TestNullptr {
        static void foo(int x) { std::cout << __func__ << ": " << x << std::endl; }
        static void foo(char *x) { std::cout << __func__ << ": " << static_cast<void*>(x) << std::endl; }
    };

    TestNullptr::foo(0);
    // TestNullptr::foo(NULL);
    TestNullptr::foo(nullptr);
}
