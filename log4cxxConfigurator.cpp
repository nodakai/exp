#include <cstdio>
#include <cstdlib>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>

#include <boost/thread.hpp>

using namespace std;
using namespace log4cxx;
using namespace boost;

void threadKernel(int tid)
{
    char buf[64];
    snprintf(buf, sizeof buf, "Thread-%d", tid);
    LoggerPtr logger(Logger::getLogger(buf));

    for (int i = 0; i < tid; ++i)
        LOG4CXX_INFO(logger, "Hi there");
}

LoggerPtr s_logger(Logger::getLogger("Main"));


int main(int argc, char *argv[])
{
    int nThreads = -1;
    if (argc > 1)
        nThreads = atoi(argv[1]);

    if (argc > 2)
        xml::DOMConfigurator::configure(argv[2]);
    else
        BasicConfigurator::configure();

    LOG4CXX_INFO(s_logger, "Hi there");

    if (nThreads >= 0) {
        thread_group tgr;
        for (int i = 0; i < nThreads; ++i) {
            tgr.create_thread(bind(threadKernel, i));
        }
        tgr.join_all();
    }
}
