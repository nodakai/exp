#include <map>
#include <iostream>
#include <string>
#include <utility>

using namespace std;

int main()
{
#if 0
    typedef multimap<int, string> Map;
    Map map;

    map.insert(make_pair(1, "one"));
    map.insert(make_pair(1, "eins"));
    map.insert(make_pair(1, "ichi"));

    map.insert(make_pair(2, "two"));
    map.insert(make_pair(2, "zwei"));
    map.insert(make_pair(2, "ni"));

    for (Map::const_iterator it(map.begin()), itEnd(map.end()); it != itEnd; ++it)
        cout << it->first << ": \"" << it->second << "\"" << endl;

    cout << "find(1)" << endl;
    for (Map::const_iterator it(map.find(1)), itEnd(map.end()); it != itEnd; ++it)
        cout << it->first << ": \"" << it->second << "\"" << endl;

    cout << "find(2)" << endl;
    for (Map::const_iterator it(map.find(2)), itEnd(map.end()); it != itEnd; ++it)
        cout << it->first << ": \"" << it->second << "\"" << endl;
#endif
    typedef multimap<int, int> Map;
    Map map;

    map.insert(make_pair(1, 300));
    map.insert(make_pair(1, 200));
    map.insert(make_pair(1, 100));
    map.insert(make_pair(1, 400));
    map.insert(make_pair(1, 500));

    cout << "all elements" << endl;
    for (Map::const_iterator it(map.begin()), itEnd(map.end()); it != itEnd; ++it)
        cout << it->first << ": " << it->second << endl;

    cout << "equal_range" << endl;
    pair<Map::const_iterator, Map::const_iterator> range(map.equal_range(1));
    for (Map::const_iterator it(range.first); it != range.second; ++it)
        cout << it->first << ": " << it->second << endl;
}
