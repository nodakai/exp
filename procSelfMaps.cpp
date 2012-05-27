#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ifstream ifs("/proc/self/maps");
    string line;
    while (ifs) {
        getline(ifs, line);
        cout << line << endl;
    }

    char *p = new char;
    cout << "char = " << (void*)p << endl;
    delete p;
    cout << "literal = " << (void*)"literal" << endl;
    cout << "main = " << (void*)main << endl;
    cout << "on-stack = " << (void*)&p << endl;
}
