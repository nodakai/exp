#include <iostream>
#include <string>

using namespace std;

class C {
  public:
    string m_s;

    C (const string &s) : m_s(s)
    { }
};

int main(int argc, char *argv[]){
#if defined(BAD)
  C x( string() );
#elif defined(BAD1)
  C x( string );
#elif defined(BAD2)
  C x( (string) );
#else
  C x( (string()) );
#endif
  cout << "[" << x.m_s << "]" << endl;
  return 0;
}
