#include <iostream>
#include <fstream>
#include <string>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>

int main(int argc, char *argv[]) {
  boost::iostreams::filtering_istreambuf b;
  boost::iostreams::gzip_decompressor x;
  b.push(x);
  std::ifstream ifs(argv[1]);
  b.push(ifs);

  std::istream ifs1(&b);
  std::string line;
  while (std::getline(ifs1, line)) {
    std::cout << '[' << line << ']' << std::endl;
  }
}
