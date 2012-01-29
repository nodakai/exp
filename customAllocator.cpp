#include <iostream>
#include <string>
#include <memory>

using namespace std;

namespace nodakai {

template <typename T>
class MyAllocator : public std::allocator<T>
{
public:
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

template <typename T>
class MyStatefulAllocator : public std::allocator<T>
{
public:
    int *m_cnt;

    MyStatefulAllocator() : base_type(), m_cnt(NULL) {
        cout << __PRETTY_FUNCTION__ << ": this == " << this << "; m_cnt == " << *m_cnt << endl;
    }

    MyStatefulAllocator(int *cnt) : base_type(), m_cnt(cnt) {
        *m_cnt = 1;
        cout << __PRETTY_FUNCTION__ << ": this == " << this << "; m_cnt == " << *m_cnt << endl;
    }

    MyStatefulAllocator(const MyStatefulAllocator &o) : base_type(o), m_cnt(o.m_cnt) {
        *m_cnt += 1;
        cout << __PRETTY_FUNCTION__ << ": this == " << this << "; m_cnt == " << *m_cnt << endl;
    }

    ~MyStatefulAllocator() {
        *m_cnt -= 1;
        cout << __PRETTY_FUNCTION__ << ": this == " << this << "; m_cnt == " << *m_cnt << endl;
    }

    template <typename U>
    struct rebind { typedef MyStatefulAllocator<U> other; };

    bool operator== (const MyStatefulAllocator &o) const {
        return m_cnt == o.m_cnt;
    }
};

} // namespace nodakai

typedef basic_string<char, char_traits<char>, nodakai::MyStatefulAllocator<char> > MyString;

int main(int argc, char *argv[])
{
    auto_ptr<int> pCnt(new int);
    nodakai::MyStatefulAllocator<char> alloc(pCnt.get());
    {
        MyString foo("abc", alloc);
    }
    return 0;
}
