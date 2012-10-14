#include <iostream>
#include <map>
#include <tr1/memory>
#include <typeinfo>

using namespace std;

int main()
{
    map<int, tr1::shared_ptr<string> > m;

    for (int i = 0; i < 3; ++i) {
        // auto v = m[10];
        auto &v = m[10];

        if ( ! v) {
            cout << v << " : " << typeid(v).name() << " : " << typeid(v.get()).name() << " : " << typeid(*v).name() << endl;
            v.reset(new string("abc"));
            cout << "Reset! : " << v << " : " << v.get() << endl;
        } else {
            cout << __LINE__ << ": *v == " << *v << endl;
        }

        *v = *v + *v;
        cout << __LINE__ << ": *v == " << *v << endl;
    }

    cout << __LINE__ << ": " << m[10] << endl;
}
