#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstddef>

using namespace std;

class ArrayMap {
private:
    static const int DEFAULT_WIDTH = 10;
    enum RangeCheckRes {
        Ok, TooSmall, TooLarge
    };

    int m_minKey, m_maxKey;
    typedef std::vector<int> Vec;
    Vec m_vec;

    void dump() {
        cout << "---V---" << endl;
        for (int i = maxKey(); i >= minKey(); --i) {
            cout << i << ": " << m_vec[i - minKey()] << endl;
        }
        cout << "---^---" << endl;
    }

public:
    ArrayMap(int basePt)
        : m_minKey(basePt - DEFAULT_WIDTH), m_maxKey(basePt + DEFAULT_WIDTH),
        m_vec(2 * DEFAULT_WIDTH + 1)
    { }

    int minKey() const { return m_minKey; }
    int maxKey() const { return m_maxKey; }

    void repack()
    {
        // dump();

        const int oWidth = (m_vec.size() - 1) / 2, nWidth = 2 * oWidth; // mean half widths
        Vec v(2 * nWidth + 1);
        copy(m_vec.begin(), m_vec.end(), v.begin() + oWidth);
        // copy(cpStart, cpStart + m_vec.size(), m_vec.begin());
        swap(m_vec, v);
        m_minKey -= oWidth; m_maxKey += oWidth;
    }

    int get(int k) const
    {
        if (minKey() <= k && k <= maxKey())
            return m_vec[k - minKey()];
        else return 0;
    }

    void set(int k, int v)
    {
        if (minKey() <= k && k <= maxKey())
            m_vec[k - minKey()] = v;
        else {
            repack();
            set(k, v);
        }
    }
};

typedef map<int, int> Map;

static void doTest(ArrayMap &am, Map &m, int k, int v)
{
    cout << "Updating with (" << k << ", " << v << ")" << endl;
    am.set(k, am.get(k) + v);
    m[k] += v;

    set<int> visited;
    for (int i = 0; i < 10000; ++i) {
        if (visited.end() != visited.find(i)) continue;

        visited.insert(i);
        const int kk = ::rand() % (am.maxKey() * 2);
        const int r1 = am.get(kk), r2 = m[kk];
        if (r1 != r2) {
            cerr << "For " << kk << ": am -> " << r1 << " != " << r2 << " <- m" << endl;
        }
    }
}

int main(int argc, char *argv[])
{
    const int n = argv[1] ? ::atoi(argv[1]) : 10;
    const int m = argv[2] ? ::atoi(argv[2]) : 10;

    for (int j = 0; j < m; ++j) {
        cout << "Initialization" << endl;
        ArrayMap am(30);
        Map m;

        const int n = argv[1] ? ::atoi(argv[1]) : 10;
        for (int i = 0; i < n; ++i) {
            const int k = ::rand() % 60;
            const int v = ::rand() % 100;
            doTest(am, m, k, v);
        }
    }
}
