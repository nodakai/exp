#include <iostream>
#include <map>
#include <utility>

using namespace std;

int main()
{
    typedef map<int, pair<int,int> > Map;
    Map m;
    m[10] = make_pair(100, 1000);
    m[20] = make_pair(200, 2000);
    Map::iterator it(m.begin());
    it->second.first = 102;
    it->second.second = 1010;

    for (Map::const_iterator it(m.begin()), itEnd(m.end()); it != itEnd; ++it)
        cout << it->first << " -> (" << it->second.first << "," << it->second.second << ")" << endl;
}
