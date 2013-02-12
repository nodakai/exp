#include <iostream>
#include <mutex>

using namespace std;

#include <unistd.h>
#include <sys/types.h>

class UniqResource {
private:
    int m_c;
    std::mutex m_mtx;
    typedef std::unique_lock<std::mutex> Lock;

public:
    UniqResource() : m_c{ 0 } { }

    class Handle {
    private:
        UniqResource *m_p;
        Lock m_lk;

    public:
        Handle(UniqResource *p, Lock lk)
            : m_p{ p }, m_lk{ std::move(lk) }
        { }

        UniqResource &operator*() { return *m_p; }
        UniqResource *operator->() { return m_p; }
    };

    Handle getLock() {
        return Handle{ this, Lock(m_mtx) };
    }

    int getCount() {
        getuid(); // to trace with strace(1)
        return m_c++;
    }
};

class User {
private:
    UniqResource &m_r;

public:
    explicit User(UniqResource &r) : m_r(r) { }

    UniqResource::Handle getResourceHdl() { return m_r.getLock(); }

    int foo() {
        auto h = getResourceHdl();
        return h->getCount();
    }
};

int main()
{
    UniqResource r;
    User u{ r };
    int x = u.foo();
    int y = u.foo();
    int z = u.foo();
    cout << "x == " << x << ", y == " << y << ", z == " << z << endl;
}
