#include <iostream>
#include <chrono>

#include <time.h>
#include <sys/time.h>

using std::cout;
using std::endl;

int main()
{
    std::chrono::monotonic_clock::time_point nowPt = std::chrono::monotonic_clock::now();

    ::time_t nowTime_t = std::chrono::monotonic_clock::to_time_t( nowPt );
    cout << ::ctime(&nowTime_t) ;
    cout << "sizeof(nowPt)==" << sizeof(nowPt) << ";  sizeof(nowTime_t)==" << sizeof(nowTime_t) <<
        "; sizeof(timeval)==" << sizeof(timeval) << endl;

    std::chrono::monotonic_clock::time_point oldPt = nowPt;
    nowPt = std::chrono::monotonic_clock::now();

    std::chrono::duration<double> diff = std::chrono::duration_cast<std::chrono::duration<double>>(nowPt - oldPt);
    cout << 1e6 * diff.count() << " usec" << endl;
}
