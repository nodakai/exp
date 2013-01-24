#include <iostream>
#include <sstream>

using namespace std;

#include <cstdlib>
#include <ctime>

static inline long diffTsNsec(const ::timespec &lhs, const ::timespec &rhs)
{
    const long giga = 1000L * 1000 * 1000;
    return giga * (lhs.tv_sec - rhs.tv_sec) + (lhs.tv_nsec - rhs.tv_nsec);
}

int main()
{
    const size_t nRep = 1000 * 1000;
    ::timespec ts0, ts1;

    ostringstream oss;
    for (int i = 0; i < 10; ++i)
        oss << __FILE__ << __DATE__ << __TIME__ << __PRETTY_FUNCTION__ ;

    int sum = 0;
    for (size_t i = 0; i < 10 + nRep; ++i) {
        if (i == 10)
            ::clock_gettime(CLOCK_MONOTONIC, &ts0);
        oss.str("");
        for (int i = 0; i < 5; ++i)
            oss << __FILE__ << __DATE__ << __TIME__ << __PRETTY_FUNCTION__ ;
        const string &tmp = oss.str();
        const string &msg = string("[") + tmp + "]" ;
        sum += msg.size();
    }
    ::clock_gettime(CLOCK_MONOTONIC, &ts1);

    const long diffNsec = diffTsNsec(ts1, ts0);
    cout << "Elapsed time is " << (1e-3 * diffNsec) << " usec (" << (static_cast<double>(diffNsec) / nRep) << " nsec per call.)" << endl;
    cout << "sum == " << sum << ", sum/nRep == " << (sum / nRep) << endl;
}
