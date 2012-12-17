#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>

using namespace std;

#include <unistd.h>
#include <getopt.h>
#include <time.h>

static long myAtoi(const char *str)
{
    char *end;
    const long ret = ::strtol(str, &end, 10);
    if ('\0' == *end)
        return ret;
    else
        throw std::invalid_argument(string("myAtoi: invalid argument [") + str + "]");
}

static void printUsage()
{
    cout << "fsync (-f|-d|-o|-n) [-N bytes] [-k] [-h]" << endl;
    ::exit(10);
}

inline int getFd(std::ofstream &ofs)
{
    struct GetFd : basic_filebuf<char> {
        int fd() { return _M_file.fd(); }
    };
    return static_cast<GetFd *>(ofs.rdbuf())->fd();
}

enum EnumMode {
    EnumModeNosync,
    EnumModeFsync,
    EnumModeFdatasync,
    EnumModeOfstreamFlush
};

inline long diffUsec(const ::timespec &ts0, const ::timespec &ts1)
{
    return (ts0.tv_sec - ts1.tv_sec) * 1000 * 1000 +
        (ts0.tv_nsec - ts1.tv_nsec) / 1000;
}

int main(int argc, char *argv[])
{
    string outf("fsync.out.dat");
    long size = 1024;
    EnumMode mode = EnumModeNosync;
    bool kill = false;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "fdonN:kh"))) {
        switch (optchr) {
            case 'f': mode = EnumModeFsync; break;
            case 'd': mode = EnumModeFdatasync; break;
            case 'o': mode = EnumModeOfstreamFlush; break;
            case 'n': mode = EnumModeNosync; break;

            case 'k': kill = true; break;

            case 'N': size = myAtoi(::optarg); break;

            case 'h':
            default: printUsage();
        }
    }

    if (::optind < argc)
        outf.assign(argv[::optind]);

    cout << "output==\"" << outf << "\"; size==" << size << "\"" << endl;

    std::ofstream ofs(outf.c_str());
    string data(size + 1024, 'A');

    ::timespec ts0 = { }, ts1 = { };
    if (EnumModeNosync == mode) {
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ofs.write(data.data(), size);
        ::clock_gettime(CLOCK_REALTIME, &ts1);
    } else if (EnumModeFsync == mode) {
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ofs.write(data.data(), size);
        ::fsync( getFd(ofs) );
        ::clock_gettime(CLOCK_REALTIME, &ts1);
    } else if (EnumModeFdatasync == mode) {
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ofs.write(data.data(), size);
        ::fdatasync( getFd(ofs) );
        ::clock_gettime(CLOCK_REALTIME, &ts1);
    } else if (EnumModeOfstreamFlush == mode) {
        ::clock_gettime(CLOCK_REALTIME, &ts0);
        ofs.write(data.data(), size);
        ofs.flush();
        ::clock_gettime(CLOCK_REALTIME, &ts1);
    }

    const long diff = diffUsec(ts1, ts0);
    if (diff > 1000) {
        if (diff > 1000*1000)
            cout << diff*1e-6 << " sec" << endl;
        else
            cout << diff*1e-3 << " msec" << endl;
    } else {
        cout << diff << " usec" << endl;
    }

    if (kill)
        return *reinterpret_cast<int *>(0);
}
