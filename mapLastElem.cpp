#include <iostream>
#include <map>

using namespace std;

int main() {
    typedef map<int, string> Map;
    Map m;
    m[1] = "ONE";
    m[2] = "TWO";
    m[3] = "THREE";

    Map::const_iterator i0 = m.begin();
    cout << i0->first << " -> " << i0->second << endl;

    Map::const_reverse_iterator i1 = m.rbegin();
    cout << i1->first << " -> " << i1->second << endl;

    Map::const_iterator i2 = m.end();
    --i2;
    cout << i2->first << " -> " << i2->second << endl;

    Map::const_iterator i3 = --m.end();
    cout << i3->first << " -> " << i3->second << endl;
}
