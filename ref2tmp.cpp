#include <iostream>
#include <sstream>
using namespace std;

int main()
{
    ostringstream oss;
    // string &s = oss.str();
    const string &s = oss.str();
    cout << s << endl;
}
