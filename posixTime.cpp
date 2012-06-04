#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <boost/thread/thread.hpp>

using namespace std;
using namespace boost;

int main(int argc, char *argv[])
{
    posix_time::ptime now(boost::posix_time::microsec_clock::universal_time());
    cout << sizeof(now) << now << endl;
    this_thread::sleep(posix_time::milliseconds(123));
}
