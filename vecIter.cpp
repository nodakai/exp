#include <iostream>
#include <string>
#include <vector>
#include <typeinfo>

using namespace std;

int main(int argc, char *argv[])
{
    vector<string> vi;
    cout << typeid(vi).name() << ", " << sizeof(vi) << endl;
    cout << typeid(vi.begin()).name() << ", " << sizeof(vi.begin()) << endl;
    cout << typeid(vi.data()).name() << ", " << sizeof(vi.data()) << endl;
}
