#include <iostream>
#include <cstdio>

#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
  ::ifaddrs *list;
  const int rc = ::getifaddrs(&list);
  if (rc) {
    ::perror("getifaddrs");
    return 1;
  }
  std::unique_ptr< ::ifaddrs, void (*)(::ifaddrs *)> raii(list, ::freeifaddrs);

  const std::string tgt(argv[1]);

  for (auto ifa = list; ifa; ifa = ifa->ifa_next) {
    char host[INET_ADDRSTRLEN] = {0};
    if (ifa->ifa_addr && AF_INET == ifa->ifa_addr->sa_family && ::inet_ntop(AF_INET, &reinterpret_cast<const ::sockaddr_in *>(ifa->ifa_addr)->sin_addr, host, sizeof host)) {
      cout << host << endl;
      if (host == tgt) {
        cout << "ifa_name==[" << ifa->ifa_name << "]" << endl;
      }
    }
  }

}
