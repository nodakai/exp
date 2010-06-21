#include <iostream>
#include <memory>

using namespace std;

struct S
{
    string s_;
    S(const string &s) : s_(s) {
        cout << "ctor: " << s_ << endl;
    }
    S(const S &o) : s_(o.s_) {
        cout << "copy ctor: " << s_ << endl;
    }
    ~S() {
        cout << "dtor: " << s_ << endl;
    }
};

S makeS02(void)
{
    S s01("01");
    cout << "01-02" << endl;
    S s02("02");
    cout << "02 a" << endl;
    return s02;
}

#define DECL(ln) DECL_aux(ln)
#define DECL_aux(ln) S s##ln(#ln)
int main(int argc, char *argv)
{
    cout << "Test 0" << endl;
    {
        S s01("01");
        {
            S s02("02");
        }
        S s03("03");
    }

    cout << endl << "Test 1" << endl;
    {
        S *s01 = new S("01");
        cout << "01-02" << endl;
        S *s02 = new S("02");
        cout << "02 a" << endl;
        delete s01;
        cout << "02 b" << endl;
        delete s02;
    }

    cout << endl << "Test 2" << endl;
    {
        S s02(makeS02());
        cout << "02 b" << endl;
    }

    return 0;
}
