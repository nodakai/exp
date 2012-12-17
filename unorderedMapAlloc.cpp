#include <iostream>
#include <string>
#include <tr1/unordered_map>

using namespace std;

#include <getopt.h>

struct Foo {
    int x;
    int y;
    double z;

    Foo() = default;
};

int main(int argc, char *argv[])
{
    tr1::unordered_map<int, Foo> i2f(1000);
}
