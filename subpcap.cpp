#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <memory>

using namespace std;

#include <stdint.h>

#include <sys/time.h> // ::timeval
#include <netinet/in.h> // ntohl
#include <getopt.h>

const long kilo = 1000, mega = kilo * kilo, giga = kilo * mega;

static void printUsage()
{
    cout << "subpcap -f [YYYY/mm/dd:]HH:MM:SS[.mmm] [-t [YYYY/mm/dd:]HH:MM:SS[.mmm]] input.pcap output.pcap" << endl;
}

struct __attribute__((packed)) pcap_file_header {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;	/* gmt to local correction */
    uint32_t sigfigs;	/* accuracy of timestamps */
    uint32_t snaplen;	/* max length saved portion of each pkt */
    uint32_t linktype;	/* data link type (LINKTYPE_*) */
};

struct __attribute__((packed)) pcap_pkthdr {
    uint32_t tv_sec;	/* time stamp */
    uint32_t tv_usec;
    uint32_t caplen;	/* length of portion present */
    uint32_t len;	/* length this packet (off wire) */
};

static ::timeval parseTimeSpec(const string &time) {
    // yyyy.mm.dd.HH:MM:SS.mmm
    // 01234567890123456789012

    ::time_t theTime_t = ::time(NULL);
    ::tm theTm = { };
    ::localtime_r(&theTime_t, &theTm);

    int HH, MM, SS, mmm;
    char delim;
    istringstream iss(time);

    if (18 <= time.size()) {
        if (23 == time.size()) {
            int yyyy;
            iss >> yyyy >> delim ;
            theTm.tm_year = yyyy - 1900;
        }
        int mm, dd;
        iss >> mm >> delim ;
        iss >> dd >> delim ;
        theTm.tm_mon = mm - 1;
        theTm.tm_mday = dd;
    }

    iss >> HH >> delim ;
    iss >> MM >> delim ;
    iss >> SS >> delim ;
    iss >> mmm ;
    if ( ! iss)
        throw std::invalid_argument(string("Invalid time \"") + time + "\"");

    theTm.tm_hour = HH;
    theTm.tm_min = MM;
    theTm.tm_sec = SS;

    const ::timeval ret = { ::mktime(&theTm), mmm * 1000 };
    cout << ::ctime(&ret.tv_sec) ;
    return ret;
}

static ::timeval add100ms(::timeval toTime) {
    long addUsec = 100 * kilo;
    toTime.tv_sec += addUsec / mega;
    addUsec %= mega;

    toTime.tv_usec += addUsec;
    toTime.tv_sec += toTime.tv_usec / mega;
    toTime.tv_usec %= mega;

    return toTime;
}

static void adjust(::timeval &adjusted, const ::timeval &ref)
{
    ::tm am0Tm;
    ::localtime_r(&ref.tv_sec, &am0Tm);
    am0Tm.tm_hour = am0Tm.tm_min = am0Tm.tm_sec = 0;
    ::time_t am0Time_t = ::mktime(&am0Tm);
    adjusted.tv_sec += am0Time_t;
}

inline bool operator<(const ::timeval a, const ::timeval b)
{
    if (a.tv_sec != b.tv_sec)
        return a.tv_sec < b.tv_sec;
    else
        return a.tv_usec < b.tv_usec;
}

inline bool operator==(const ::timeval a, const ::timeval b)
{
    return a.tv_sec == b.tv_sec && a.tv_usec && b.tv_usec;
}

inline bool operator<=(const ::timeval a, const ::timeval b)
{
    return a == b || a < b;
}

static void doProcess(const string &fromTimeStr, const string &toTimeStr, ifstream &ifs, ofstream &ofs)
{
    ::timeval fromTime(parseTimeSpec(fromTimeStr));
    ::timeval toTime(toTimeStr.empty() ?
        add100ms(fromTime) :
        parseTimeSpec(toTimeStr));

    char buf[1 << 16];

    ifs.read(buf, sizeof(pcap_file_header));
    if ( ! ifs)
        throw std::runtime_error("Couldn't read the PCAP file header.");
    const pcap_file_header * const fh = reinterpret_cast<const pcap_file_header *>(buf);
    if (0xA1B2C3D4 != fh->magic)
        throw std::runtime_error("Wrong magic number found in the PCAP file header.");

    ofs.write(buf, sizeof *fh);
    if ( ! ofs)
        throw std::runtime_error("Failed to write the PCAP file header.");

    size_t cnt = 0;
    for (;; ++cnt) {
        ifs.read(buf, sizeof(pcap_pkthdr));
        if ( ! ifs)
            break;;

        const pcap_pkthdr * const ph = reinterpret_cast<const pcap_pkthdr *>(buf);
        const uint32_t caplen = ph->caplen;
        if (sizeof buf <= caplen)
            throw std::runtime_error("Wrong caplen");
        const ::timeval capTime = { ph->tv_sec, ph->tv_usec };

        if (fromTime.tv_sec < 60*60*24) {
            adjust(fromTime, capTime);
            adjust(toTime, capTime);
        }

        if (toTime < capTime)
            break;

        if (fromTime <= capTime) {
            ofs.write(buf, sizeof *ph);
            if ( ! ofs)
                throw std::runtime_error("Failed to write the PCAP packet header.");

            ifs.read(buf, caplen);
            if ( ! ifs)
                throw std::runtime_error("Failed to read the captured data");

            ofs.write(buf, caplen);
            if ( ! ofs)
                throw std::runtime_error("Failed to write the captured data.");
        } else {
            ifs.ignore(caplen);
        }
    }

    cout << "cnt == " << cnt << endl;
}

int main(int argc, char *argv[]) {
    string fromTime, toTime;
    const char *inFile = NULL, *outFile = NULL;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "f:t:h:"))) {
        switch (optchr) {
            case 'f': fromTime.assign(::optarg); break;
            case 't': toTime.assign(::optarg); break;

            case 'h': default: printUsage();
        }
    }

    for (int i = ::optind; i < argc; ++i) {
        if (NULL == inFile)
            inFile = argv[i];
        else if (NULL == outFile)
            outFile = argv[i];
    }
    if (fromTime.empty() || NULL == outFile)
        printUsage();

    cout << "inFile==\"" << inFile << "\", outFile==\"" << outFile << "\"" << endl;

    ifstream ifs(inFile, ios::binary);
    ofstream ofs(outFile, ios::out | ios::binary);

    doProcess(fromTime, toTime, ifs, ofs);
}
