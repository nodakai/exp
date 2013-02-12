#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    const char c = 200;
    const int x = 100;

    cout << "c==[" << std::hex << std::setw(8) << std::setfill('0') << (unsigned)(unsigned char)c << ']' << std::dec << std::setfill(' ') << endl;

    cout << "x==[" << std::setw(8) << x << ']' << endl;
}
