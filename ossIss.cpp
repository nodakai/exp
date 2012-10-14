#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

static void test01()
{
    const int WIDTH = 10;

    ostringstream oss;
    oss << setw(WIDTH) << 123 << setw(WIDTH) << 456;
    for (int i = 0; i < 10; ++i)
        oss << " (^o^) " ;

    const string &s = oss.str();
    cout << "s == [" << s << "]" << endl;

    int x, y;
    istringstream iss(s);
    iss >> x >> y;

    cout << "x == " << x << endl;
    cout << "y == " << y << endl;
}

static void test02()
{
    const int WIDTH = 3;

    ostringstream oss;
    oss << setw(WIDTH) << 123 << setw(WIDTH) << 456;

    const string &s = oss.str();
    cout << "s == [" << s << "]" << endl;

    int x, y;
    istringstream iss(s);
    iss >> setw(WIDTH) >> x >> setw(WIDTH) >> y;

    cout << "x == " << x << endl;
    cout << "y == " << y << " !!!" << endl;
}

int main()
{
    test01();
    test02();
}
