#include <iostream>
#include <string>

using namespace std;

int main()
{
    string s;
    const char *cp = "foo";
    s.assign(cp, 10);
    cout << "\"" << s << "\"" << endl;
}
