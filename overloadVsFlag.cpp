#define _POSIX_C_SOURCE 199309L
#include <time.h>

#include<iostream>
#include<string>

using namespace std;

struct LeftAligned {
    private:
        int m_x;
    public:
        explicit LeftAligned(int x) : m_x(x) { }
        int get() const { return m_x; }
};

struct RightAligned {
    private:
        int m_x;
    public:
        explicit RightAligned(int x) : m_x(x) { }
        int get() const { return m_x; }
};

class Symbol {
    private:
        char m_s[8];
        static char s_tmp[8 + 1];

    private:
        void setLeft(int x) {
            sprintf(s_tmp, "%-8d", x);
            strncpy(m_s, s_tmp, sizeof m_s);
        }
        void setRight(int x) {
            sprintf(s_tmp, "%8d", x);
            strncpy(m_s, s_tmp, sizeof m_s);
        }
    public:
        void set(LeftAligned x) { setLeft(x.get()); }
        void set(RightAligned x) { setRight(x.get()); }
        void set(int x, int align) {
            if (align > 0)
                setLeft(x);
            else
                setRight(x);
        }
        void print() const {
            cout << "Symbol[";
            for (size_t i=0; i<sizeof m_s + 1; ++i)
                cout << m_s[i];
            cout << "]" << endl;
            for (size_t i=0; i<sizeof m_s + 1; ++i)
                cout << static_cast<unsigned>(m_s[i]) << " ";
            cout << endl;
        }
};

char Symbol::s_tmp[8 + 1];

int main(int argc, char *argv[])
{
    {
        Symbol sym;
        sym.set(LeftAligned(100));
        sym.print();
        sym.set(RightAligned(777));
        sym.print();
        sym.set(1234, 10);
        sym.print();
        sym.set(5678, 0);
        sym.print();
    }

    const int N = argv[1] ? atoi(argv[1]) : 1000 * 1000;
    for (int i=0; i<4; ++i) {
        struct timespec ts0, ts1, ts2;
        Symbol sym;
        const LeftAligned l(100);

        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts0);
        for (int k=0; k<N; ++k) {
            sym.set(l);
        }
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts1);
        for (int k=0; k<N; ++k) {
            sym.set(100, 1);
        }
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts2);

        const double dt0 = (ts1.tv_sec - ts0.tv_sec) +
            1e-9 * (ts1.tv_nsec - ts0.tv_nsec);
        const double dt1 = (ts1.tv_sec - ts0.tv_sec) +
            1e-9 * (ts1.tv_nsec - ts0.tv_nsec);
        cout << "dt0 == " << dt0 << " sec; dt1 == " << dt1 << " sec" << endl;
    }

    return 0;
}
