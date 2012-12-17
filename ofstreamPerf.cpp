#include <iostream>
#include <fstream>

using namespace std;

#include <time.h>

long long diffTimespecUsec(const ::timespec &a, const ::timespec &b)
{
    return (a.tv_sec - b.tv_sec) * 1000 * 1000 + (a.tv_nsec - b.tv_nsec) / 1000;
}

static ::timespec ts0, ts1;
static long long d0, d1, d2, d3, d4, d5, d6, d7;

#define PRT(x) cout << #x " == " << x << " us." << endl

static void test01()
{
__asm__ __volatile__ ("# NOP" : : );
    {
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ifstream ifs("/home/nodakai/.vimrc");
        ::clock_gettime(CLOCK_REALTIME, &ts1);
        d0 = diffTimespecUsec(ts1, ts0);
    }
__asm__ __volatile__ ("# NOP" : : );

    {
        ifstream ifs;
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ifs.open("/home/nodakai/.vimrc");
        ::clock_gettime(CLOCK_REALTIME, &ts1);
        d1 = diffTimespecUsec(ts1, ts0);
    }
__asm__ __volatile__ ("# NOP" : : );

    {
        ifstream ifs("/home/nodakai/.vimrc");
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ifs.close();
        ::clock_gettime(CLOCK_REALTIME, &ts1);
        d2 = diffTimespecUsec(ts1, ts0);
    }
__asm__ __volatile__ ("# NOP" : : );

    {
        {
            ifstream ifs("/home/nodakai/.vimrc");
            ::clock_gettime(CLOCK_REALTIME, &ts0);
        }
        ::clock_gettime(CLOCK_REALTIME, &ts1);
        d3 = diffTimespecUsec(ts1, ts0);
    }
__asm__ __volatile__ ("# NOP" : : );

    PRT(d0);
    PRT(d1);
    PRT(d2);
    PRT(d3);
}

#define GET(ts) ::clock_gettime(CLOCK_REALTIME, &ts)

static void test02()
{
    string datA(200, 'A'), datB(2000, 'B');

    {
        GET(ts0);
        {
            ofstream ofs("ofstreamPerfA.out", std::ios::out | std::ios::app );
            ofs << datA;
        }
        GET(ts1);
        d0 = diffTimespecUsec(ts1, ts0);
    }

    {
        GET(ts0);
        {
            ofstream ofs("/dev/shm/ofstreamPerfA.out", std::ios::out | std::ios::app );
            ofs << datA;
        }
        GET(ts1);
        d1 = diffTimespecUsec(ts1, ts0);
    }

    {
        GET(ts0);
        {
            ofstream ofs("ofstreamPerfA.out", std::ios::out | std::ios::app );
            ofs << datA;
        }
        GET(ts1);
        d0 = diffTimespecUsec(ts1, ts0);
    }


    {
        GET(ts0);
        {
            ofstream ofs("ofstreamPerfB.out", std::ios::out | std::ios::app );
            ofs << datB;
        }
        GET(ts1);
        d2 = diffTimespecUsec(ts1, ts0);
    }

    {
        GET(ts0);
        {
            ofstream ofs("/dev/shm/ofstreamPerfB.out", std::ios::out | std::ios::app );
            ofs << datB;
        }
        GET(ts1);
        d3 = diffTimespecUsec(ts1, ts0);
    }

    PRT(d0);
    PRT(d1);
    PRT(d2);
    PRT(d3);
}

int main()
{
    const int N = 3;

    for (int i = 1; i <= 100; ++i)
        delete[] (new int[1000 * i]);

    for (int i = 0; i < N; ++i) {
        // test01();

        test02();

        cout << endl;
    }
}
