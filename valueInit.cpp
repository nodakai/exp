#include <iostream>

using namespace std;

struct Foo {
    int x;
    int y;
};

struct Bar {
    static Foo f1;
    static Foo f2;
};

Foo Bar::f1 = Foo();
Foo Bar::f2;

static int si;

int main()
{
    cout << Bar::f1.x << endl;
    cout << Bar::f1.y << endl;
    cout << endl;
    cout << Bar::f2.x << endl;
    cout << Bar::f2.y << endl;
    cout << endl;

    Foo f;
    cout << f.x << endl;
    cout << f.y << endl;
    cout << endl;

    Foo ff = Foo();
    cout << ff.x << endl;
    cout << ff.y << endl;
    cout << endl;

    cout << si << endl;
}
