#include <vector>
#include <cstdio>
#include <boost/shared_ptr.hpp>

using namespace std;
namespace b = boost;

typedef vector<b::shared_ptr<int> > Vector;

int main()
{
    Vector v;
    printf("%s\n", typeid(v).name());
}
