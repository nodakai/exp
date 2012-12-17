#include <iostream>

using namespace std;

template <class T>
struct Tmpl {
    void doIt() {
        helper();
    }

    void helper();
};

struct Foo;

template <>
void Tmpl<Foo>::helper() {
    cout << __PRETTY_FUNCTION__ << ": I'm FOOOOO" << endl;
}

struct Foo {
};

struct Bar {
};

struct Baz {
};

struct Quux {
};

template <>
void Tmpl<Bar>::helper() {
    cout << __PRETTY_FUNCTION__ << ": bar" << endl;
}

template <class T>
void Tmpl<T>::helper() {
    cout << __PRETTY_FUNCTION__ << endl;
}

int main()
{
    Tmpl<Foo> tfoo;
    tfoo.doIt();

    Tmpl<Bar> tbar;
    tbar.doIt();

    Tmpl<Baz> tbaz;
    tbaz.doIt();

    Tmpl<Quux> tquux;
    tquux.doIt();
}
