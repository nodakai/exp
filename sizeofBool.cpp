#include <iostream>

using namespace std;

struct Foo {
    int x;
    int y;
};

struct Bar {
    bool b;
    Foo f;
};

struct Bar1 {
    bool b;
    bool b1;
    Foo f;
};

struct Bar2 {
    bool b;
    bool b1;
    bool b2;
    Foo f;
};

struct Bar3 {
    bool b;
    bool b1;
    bool b2;
    bool b3;
    Foo f;
};

struct Bar4 {
    bool b;
    bool b1;
    bool b2;
    bool b3;
    bool b4;
    Foo f;
};

#define PRINT(expr) cout << #expr " = " << expr << endl;
int main()
{
    PRINT(sizeof(bool));
    PRINT(sizeof(Foo));
    PRINT(sizeof(Bar));
    PRINT(sizeof(Bar1));
    PRINT(sizeof(Bar2));
    PRINT(sizeof(Bar3));
    PRINT(sizeof(Bar4));
}
