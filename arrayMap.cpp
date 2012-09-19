#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstddef>

#include <stdint.h>

using namespace std;

struct Foo {
public:
    uint8_t x[4];
    uint8_t y[4];

    Foo(int v = 0) {
        char buf[sizeof(x) + 1];
        snprintf(buf, sizeof(buf), "%4d", v);
        copy(buf, buf+sizeof x, x);
        copy(buf, buf+sizeof x, y);
    }

    operator int() const {
        char buf[sizeof(x) + 1];
        copy(x, x + sizeof x, buf);
        buf[sizeof x] = '\0';
        int x;
        sscanf(buf, "%d", &x);
        return x;
    }

    bool empty() const {
        return string("   0") == string(reinterpret_cast<const char *>(x), sizeof x);
    }
};

ostream &operator<<(ostream &os, const Foo &o)
{
    return os << "Foo { " << string(reinterpret_cast<const char *>(o.x), 4) << ", " << string(reinterpret_cast<const char *>(o.y), 4) << " }" ;
}

template<typename T>
class ArrayMap {
public:
    static const int LEN_MSG_LEN = 8;

public:
// private:
    static const int DEFAULT_WIDTH = 10;
    static const int INVALID_RANGE = -1;

    static const T zero;

    int m_minKey, m_maxKey;
    typedef std::vector<T> Vec;
    Vec m_vec;

    void dump() {
        std::cout << "---V---" << std::endl;
        for (int i = maxKey(); i >= minKey(); --i) {
            std::cout << i << ": " << m_vec[i - offset()] << std::endl;
        }
        std::cout << 0 << ": " << m_vec[0] << std::endl;
        std::cout << "---^---" << std::endl;
    }

    int minKey() const { return m_minKey; }
    int offset() const { return m_minKey - 1; }
    int maxKey() const { return m_maxKey; }

    void repack() {
        // dump();

        const int oWidth = (m_vec.size() - 1 - 1) / 2, nWidth = 2 * oWidth; // half widths
        Vec v(2 * nWidth + 1 + 1);
        copy(m_vec.begin() + 1, m_vec.end(), v.begin() + oWidth + 1);
        if ( ! m_vec.empty())
            v[0] = m_vec[0];
        swap(m_vec, v);
        m_minKey -= oWidth; m_maxKey += oWidth;
    }

public:
    ArrayMap(int basePt)
        : m_minKey(basePt - DEFAULT_WIDTH), m_maxKey(basePt + DEFAULT_WIDTH),
        m_vec(2 * DEFAULT_WIDTH + 1 + 1)
    { }

    ArrayMap(int minKey, int maxKey)
        : m_minKey(minKey - DEFAULT_WIDTH), m_maxKey(maxKey + DEFAULT_WIDTH),
        m_vec(2 * DEFAULT_WIDTH + (maxKey - minKey) + 1 + 1)
    { }

    std::pair<int, int> keyRange() const {
        int mn = INVALID_RANGE, mx = INVALID_RANGE - 1;
        for (int i = minKey(); i <= maxKey(); ++i) {
            if ( ! get(i).empty()) {
                mn = i;
                break;
            }
        }
        for (int i = maxKey(); i >= minKey(); --i) {
            if ( ! get(i).empty()) {
                mx = i;
                break;
            }
        }
        return std::make_pair(mn, mx);
    }

    static bool empty(std::pair<int, int> p) { return INVALID_RANGE >= p.first || INVALID_RANGE >= p.second; }

    const T &get(int k = 0) const {
        if (0 == k)
            return m_vec[0];
        return (minKey() <= k && k <= maxKey()) ? m_vec[k - offset()] : zero;
    }

    void set(int k, const T &v) {
        if (minKey() <= k && k <= maxKey()) {
            m_vec[k - offset()] = v;
        } else if (0 == k) {
            m_vec[0] = v;
        } else {
            repack();
            set(k, v);
        }
    }

    void printPair(std::ostream &oss, int k) {
        oss << std::setw(LEN_MSG_LEN) << k << get(k) ;
    }

    void toString(std::ostream &oss)
    {
        const auto &range = keyRange();
        oss << std::setw(LEN_MSG_LEN) << (range.second - range.first + 1 + (get().empty() ? 0 : 1)) ;
        if ( ! get().empty())
            printPair(oss, 0);
        for (int k = range.first; k <= range.second; ++k) {
            if ( ! get(k).empty())
                printPair(oss, k);
        }
    }
};

template<>
const Foo ArrayMap<Foo>::zero = Foo();

typedef map<int, Foo> Map;

static void doTest(ArrayMap<Foo> &am, Map &m, int k, int v)
{
    cout << "Updating with (" << k << ", " << v << ")" << endl;
    am.set(k, am.get(k) + v);
    m[k] = m[k] + v;

    set<int> visited;
    for (int i = 0; i < 10000; ++i) {
        if (visited.end() != visited.find(i)) continue;

        visited.insert(i);
        const int kk = am.minKey() + ::rand() % ((am.maxKey() - am.minKey()) * 2);
        const int r1 = am.get(kk), r2 = m[kk];
        if (r1 != r2) {
            cerr << "For " << kk << ": am -> " << r1 << " != " << r2 << " <- m" << endl;
        }
    }
}

int main(int argc, char *argv[])
{
    int n = argc > 1 ? ::atoi(argv[1]) : 10;
    const int m = argc > 2 ? ::atoi(argv[2]) : 10;

    if (n > 0) {
        for (int j = 0; j < m; ++j) {
            cout << "Initialization" << endl;
            ArrayMap<Foo> am(30);
            Map m;

            for (int i = 0; i < n; ++i) {
                const int k = (0 == ::rand() % 10) ? 0 : 10 + ::rand() % 40;
                const int v = ::rand() % 100;
                doTest(am, m, k, v);
            }
        }
    } else {
        n = -n;

        ArrayMap<Foo> am(30);
        am.dump();

        am.set(30, 1);
        am.dump();
        am.set(31, 2);
        am.dump();
        am.set(40, 4);
        am.dump();
        am.set(20, 6);
        am.dump();
        am.set(0, 10);
        am.dump();

        cout << endl;
        cout << am.get(30) << endl;
        cout << am.get(31) << endl;
        cout << am.get(40) << endl;
        cout << am.get(20) << endl;
        cout << am.get(0) << endl;

        cout << endl;
        for (size_t i = 0; i < am.m_vec.size(); ++i) {
            cout << i << ": " << am.m_vec[i] << endl;
        }

        cout << "[" ;
        am.toString(cout);
        cout << "]" << endl;
    }
}
