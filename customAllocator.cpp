#include <iostream>
#include <string>

using namespace std;

template <typename T>
class MyAllocator : public std::allocator<T>
{
public:
    typedef std::allocator<T> base_type;

    int *m_cnt;

    MyAllocator() : base_type(), m_cnt(new int) {
        *m_cnt = 1;
        cout << __PRETTY_FUNCTION__ << ": this == " << this << "; m_cnt == " << *m_cnt << endl;
    }

    MyAllocator(const MyAllocator &o) : base_type(o), m_cnt(o.m_cnt) {
        *m_cnt += 1;
        cout << __PRETTY_FUNCTION__ << ": this == " << this << "; m_cnt == " << *m_cnt << endl;
    }

    ~MyAllocator() {
        *m_cnt -= 1;
        cout << __PRETTY_FUNCTION__ << ": this == " << this << "; m_cnt == " << *m_cnt << endl;
    }

    template <typename U>
    struct rebind { typedef MyAllocator<U> other; };
};

typedef basic_string<char, char_traits<char>, MyAllocator<char> > MyString;

int main(int argc, char *argv[])
{
    MyAllocator<char> alloc;
    {
        MyString foo("abc", alloc);
    }
    return 0;
}
