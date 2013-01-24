#include <map>

class Map : public std::map<int, Map::Foo> {
public:
    class Foo { int x; };
};

int main() { }
