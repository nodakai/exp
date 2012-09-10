#include <iostream>
#include <thread>

#include <pthread.h>

using std::cout;
using std::endl;

static void threadKernel(int x)
{
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cout << "tid==" << std::hex << std::this_thread::get_id() << std::dec << ";  x==" << x << ";  i==" << i << endl;
    }
    cout << "Finishing the thread #" << x << endl;
}

int main(int argc, char *argv[])
{
    cout << "Starting..." << endl;
    for (int k = 0; k < 5; ++k) {
        cout << "std::thread t(threadKernel, " << k << ");" << endl;
        std::thread t(threadKernel, k);
        t.detach();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    cout << "Quitting..." << endl;
    ::pthread_exit(nullptr);
}
