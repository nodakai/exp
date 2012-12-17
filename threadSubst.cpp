#include <iostream>
#include <thread>
#include <cerrno>

using namespace std;

#include <time.h>

void myMicroSleep(long microSec)
{
    const long mega = 1000 * 1000;
    ::timespec sleepDur = { microSec / mega, (microSec % mega) * 1000 }, rest;
    while (::nanosleep(&sleepDur, &rest) && EINTR == errno)
        sleepDur = rest;
}

void threadKernel(const char *name)
{
    for (int i = 0; i < 3; ++i) {
        myMicroSleep(100*1000);
        cout << name << endl;
    }
}

int main(int argc, char *argv[])
{
    std::thread t(threadKernel, argv[0]);

    myMicroSleep(500 * 1000);

    t = std::thread(threadKernel, argv[0]);

    t.join();
}
