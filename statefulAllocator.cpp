#include <iostream>
#include <memory>

#include <sys/mman.h>

using namespace std;

namespace nodakai {

class MemInfo {
public:
    static const size_t MY_OFFSET;

    unsigned m_refCnt;
    size_t m_allocatedBytes;
    char m_p[0];

    inline void initialize() {
        m_refCnt = 1;
        m_allocatedBytes = 0;
    }

    static inline size_t align(size_t x) {
        const unsigned shift = 3; // 2**3 == 8 bytes alignment
        return ( ((x - 1) >> shift) + 1 ) << shift;
    }

    inline char *incRef(int delta=1) {
        m_refCnt += delta;
        return reinterpret_cast<char *>(this);
    }

    inline unsigned decRef(int absDelta=1) { return m_refCnt -= absDelta; }

    inline size_t allocate(size_t size) {
        const size_t old = m_allocatedBytes;
        m_allocatedBytes += size;
        return old;
    }
};

const size_t MemInfo::MY_OFFSET = MemInfo::align( sizeof(MemInfo) );

//** COPIED FROM GNU libstdc++, FORCING THIS SAMPLE TO BE UNDER GPL.
template<typename T>
class allocator
{
public:
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
    typedef T *        pointer;
    typedef const T *  const_pointer;
    typedef T &        reference;
    typedef const T &  const_reference;
    typedef T          value_type;

protected:
    /*
    union UMemInfo {
    public:
        char *m_p;
        MemInfo m_mem;

        inline UMemInfo() : m_p(0) { }

        template <typename U>
        inline UMemInfo(U *p) : m_p(reinterpret_cast<char *>(p)) {
            m_mem.initialize();
        }

        inline UMemInfo(const UMemInfo &o) : m_p(o.m_p) {
            m_mem.incRef();
        }


        char *incRef(int delta=1) {
            m_mem.incRef(delta);
            return m_p;
        }

        inline char * &decRef(int absDelta=1) {
            m_mem.decRef(absDelta) || munmap(m_p, MAX_SIZE);
            return m_p;
        }
    } m_u;
    */
    char *m_p;

    static const size_t GIGA = size_t(1024) * 1024 * 1024;
    static const size_t MAX_SIZE;

public:
    template<typename U>
    struct rebind
    { typedef allocator<U> other; };

    allocator() : m_p(0) { }

    template <typename U>
    allocator(U *p) : m_p(reinterpret_cast<char *>(p)) {
        getInfo().initialize();
    }

    static void *mmap() {
        // return ::malloc(MAX_SIZE);
        return ::mmap(0, MAX_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    }

    MemInfo &getInfo() {
        return *reinterpret_cast<MemInfo *>(m_p);
    }

    template<typename U>
    allocator(const allocator<U> &o) throw() : m_p(getInfo().incRef()) { }

    void decRef() {
        const unsigned isZero = getInfo().decRef();
        // cout << "isZero == " << isZero << endl;
        if (0 == isZero)
            munmap(m_p, MAX_SIZE);
    }

    template<typename U>
    allocator<T> &operator=(allocator<T> &o) {
        decRef();
        m_p = &o.getInfo().incRef();
    }

    ~allocator() throw() {
        decRef();
    }

    pointer address(reference x) const { return &x; }

    const_pointer address(const_reference x) const { return &x; }

    // NB: __n is permitted to be 0.  The C++ standard says nothing
    // about what the return value is when __n == 0.
    pointer allocate(size_type n, const void* = 0)
    {
        if (__builtin_expect(n > this->max_size(), false))
            std::__throw_bad_alloc();

        const size_type idx = getInfo().allocate(n);
        return &getInfo().m_p[idx];
    }

    // __p is not permitted to be a null pointer.
    void deallocate(pointer, size_type) { /* nop */ }

    size_type max_size() const throw() { return MAX_SIZE; }

    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 402. wrong new expression in [some_] allocator::construct
    void construct(pointer p, const_reference val)
    { ::new(p) T(val); }

    void destroy(pointer p) { p->~T(); }

    template <typename U>
    inline bool equals(const allocator<U> &o) const {
        return m_p == o.m_p;
    }
};

template <typename T>
const size_t allocator<T>::MAX_SIZE = 16 * allocator<T>::GIGA / sizeof(T); // size_t(-1) / sizeof(T);

template <typename T, typename U>
inline bool operator==(const allocator<T> &a, const allocator<U> &b)
{ return a.equals(b); }

template <typename T, typename U>
inline bool operator!=(const allocator<T> &a, const allocator<U> &b)
{ return ! (a == b); }

} // namespace nodakai

typedef std::basic_string<char, char_traits<char>, nodakai::allocator<char> > MyString;

int main() /* (int argc, char *argv[]) */ {
    void * const p = nodakai::allocator<char>::mmap();
    // cout << "Go! p == " << p << endl;
    nodakai::allocator<char> alloc(p);
    {
        MyString foo("abcde", alloc);
        {
            MyString bar = foo;
            bar += "jugemu";
        }
    }
    return 0;
}
