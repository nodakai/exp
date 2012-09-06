#include <iostream>

#include <vector>
#include <set>
#include <map>

using namespace std;

#define DECLARE(...)           \
    cout << DECLARE_AUX((__VA_ARGS__)) << endl;      \
    __VA_ARGS__

#define DECLARE_AUX(x) #x

#define TF(b) (b ? "T" : "F")

#define COMPARE(pfx)            \
    cout << "(" #pfx "0 == " #pfx "1) == " << TF(pfx##0 == pfx##1) << ", (" #pfx "0 < " #pfx "1) == " << TF(pfx##0 < pfx##1) << ", (" #pfx "0 <= " #pfx "1) == " << TF(pfx##0 <= pfx##1) << endl

int main()
{
    {
        DECLARE(vector<int> c0, c1);
        COMPARE(c);
        c0.push_back(0);
        c1.push_back(0);
        c0.push_back(1);
        c1.push_back(1);
        c0.push_back(2);
        c1.push_back(2);
        COMPARE(c);
        c0.push_back(3);
        c1.push_back(4);
        COMPARE(c);
        c1.pop_back();
        COMPARE(c);
    }
    {
        DECLARE(set<int> c0, c1);
        COMPARE(c);
        c0.insert(0);
        c1.insert(0);
        c0.insert(1);
        c1.insert(1);
        c0.insert(2);
        c1.insert(2);
        COMPARE(c);
        c0.insert(3);
        c1.insert(4);
        COMPARE(c);
        set<int>::const_iterator c1back(c1.end());
        --c1back;
        c1.erase(c1back);
        COMPARE(c);
    }

    cout << "Quitting..." << endl;
}
