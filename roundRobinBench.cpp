#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
  std::vector<int> v;
  for (int i = 0; i < 10 + argc; ++i)
    v.push_back(i);

  const int N = 100'000'000 + argc;
  int x = 0, y = 0;
  const auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < N; ++i)
    x += v[(i + argc) % v.size()];
  const auto t1 = std::chrono::steady_clock::now();
  auto it = v.cbegin();
  std::advance(it, argc);
  const auto t2 = std::chrono::steady_clock::now();
  for (int i = 0; i < N; ++i) {
    y += *it;
    if (v.cend() == ++it)
      it = v.cbegin();
  }
  const auto t3 = std::chrono::steady_clock::now();

  const auto d0 = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
  const auto d1 = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
  cout << d0 << ", " << double(d0) / N << "ns/iter, " << x << endl;
  cout << d1 << ", " << double(d1) / N << "ns/iter, " << y << endl;
}
