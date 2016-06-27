#include <iostream>
#include <complex>
#include <cmath>

#define LIT(x) (__int128(1'000'000'000) * __int128(1'000'000'000) * (x))

static __int128 abs(std::complex<__int128> z) {
    const auto q = z.real() * z.real() / LIT(1) + z.imag() * z.imag() / LIT(1);
    const auto qd = double(q) / LIT(1);
    auto r = __int128(std::sqrt(qd) * LIT(1));
    while (q < r * r / LIT(1))
        --r;
    while (q > r * r / LIT(1))
        ++r;
    return r;
}

int main() {
    for (auto y = LIT(-1); y <= LIT(1); y += LIT(1)/15) {
        for (auto x = LIT(-2); x <= LIT(1); x += LIT(4)/100) {
            char c = 126;
            auto z = std::complex<__int128>{ x, y }, a = z;
            while (abs((z = z * z / LIT(1) + a)) < LIT(2) && (--c > 32))
                ;
            std::cout << c;
        }
        std::cout << std::endl;
    }
}
