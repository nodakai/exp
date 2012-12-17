#include <string>
#include <iostream>
#include <sstream>

using namespace std;

void test(std::istream &is) {
    string line;
    while (std::getline(is, line)) {
        cout << "[" << line << "]" << endl;
    }
}

int main() {
    {
        const string data0("abc\n" "defg\n" "hij");
        cout << "Reading from [" << data0 << "]..." << endl;
        std::istringstream iss(data0);
        test(iss);
    }

    {
        const string data1("abc\n" "defg\n" "hij\n");
        cout << "Reading from [" << data1 << "]..." << endl;
        std::istringstream iss(data1);
        test(iss);
    }
}
