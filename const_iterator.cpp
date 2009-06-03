#include <iostream>
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace boost;

typedef vector<shared_ptr<string> > vs;

void foo(const vs &lines)
{
    for (vs::const_iterator i(lines.begin()); i != lines.end(); ++i) {
        cout << **i << endl;
    }
}

int main(void)
{
    vs lines;
    lines.push_back(shared_ptr<string>(new string("abc")));
    foo(lines);

    return 0;
}
