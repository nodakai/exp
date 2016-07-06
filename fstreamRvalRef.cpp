#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]) {
  if (auto &&ifs = std::ifstream(argv[0])) {
    cout << ifs.peek() << endl;
  } else {
    cout << "Error" << endl;
  }
  return 0;
}
