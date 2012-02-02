#include <iostream>
#include <iomanip>

enum EnumDumpUntilNul { DumpUntilNul, RegardlessOfNul, };

std::ostream &operator<< (std::ostream &os, EnumDumpUntilNul x) { return os << (x == DumpUntilNul ? "DumpUntilNul" : "RegardlessOfNul"); }

#ifdef DEBUG
#define DUMP_DEBUG_PRINT(...) __VA_ARGS__
#else
#define DUMP_DEBUG_PRINT(...)
#endif

template<typename TElem>
struct Dump {
    const TElem * m_p;
    unsigned m_n;
    bool m_fixed;
    EnumDumpUntilNul m_untilNul;
    Dump(const TElem *p, unsigned n, bool fixed, EnumDumpUntilNul untilNul) : m_p(p), m_n(n), m_fixed(fixed), m_untilNul(untilNul) { }
};

template<typename TElem, unsigned N>
Dump<TElem> dump(const TElem (&buf)[N], EnumDumpUntilNul untilNul=RegardlessOfNul) {
    DUMP_DEBUG_PRINT(std::cout << "untilNul=" << untilNul << "; N(fixed)=" << N << std::endl);
    return Dump<TElem>(buf, N, true, untilNul);
}

template<typename TElem>
Dump<TElem> dump(const TElem *buf, unsigned n, EnumDumpUntilNul untilNul=RegardlessOfNul) {
    DUMP_DEBUG_PRINT(std::cout << "untilNul=" << untilNul << "; n=" << n << std::endl);
    return Dump<TElem>(buf, n, false, untilNul);
}

template<typename TElem>
std::ostream &operator<<(std::ostream &os, const Dump<TElem> &d) {
    const EnumDumpUntilNul un = d.m_untilNul;
    const int n = d.m_n;
    const bool fixed = d.m_fixed;
    const TElem * const p = d.m_p;
    os << std::hex;
    for (int i = 0; un == RegardlessOfNul && i < n || (fixed || i < n) && un == DumpUntilNul && p[i]; ++i) {
        if (i > 0) os << ' ';
        os << std::setw(2) << std::setfill('0') << static_cast<int>(p[i]);
    }
    return os << std::dec;
}

using namespace std;

int main()
{
    {
        char buf[10] = "Hello";
        cout << "[" << dump(buf) << "]" << endl;
        cout << "[" << dump(buf, 3) << "]" << endl;
        cout << "[" << dump(buf, 3, DumpUntilNul) << "]" << endl;
        cout << "[" << dump(buf, 10) << "]" << endl;
        cout << "[" << dump(buf, 10, DumpUntilNul) << "]" << endl;
    }

    cout << endl;
    {
        const char * msg = "Hello";
        cout << "[" << dump(msg, 3) << "]" << endl;
        cout << "[" << dump(msg, 3, DumpUntilNul) << "]" << endl;
        cout << "[" << dump(msg, 10) << "]" << endl;
        cout << "[" << dump(msg, 10, DumpUntilNul) << "]" << endl;
    }
}
