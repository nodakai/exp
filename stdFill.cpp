#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

#include <stdint.h>

struct Foo {
    uint8_t a[4];
    uint8_t b[4];
    uint8_t c[4];

    void set() {
        // std::fill(b, b + 1, 'X');
        std::fill(b, b + sizeof b, 'X');
    }

    string toString() const {
        const char * const this_ = reinterpret_cast<const char *>(this);
        std::ostringstream oss;
        for (size_t i = 0, iEnd = sizeof *this; i < iEnd; ++i) {
            oss << '[' << std::hex << std::setw(2) << std::setfill('0') << int(this_[i]) << ']' ;
        }
        return oss.str();
    }
};

int main()
{
    Foo f = { };

    cout << "\"" << f.toString() << "\"" << endl;

    f.set();

    cout << "\"" << f.toString() << "\"" << endl;
}
