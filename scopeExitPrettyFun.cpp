#include <iostream>
#include <stdexcept>
#include <typeinfo>

using namespace std;

static void foo(int mode)
{
    struct ScopeExit {
        const char *m_name;

    public:
        ScopeExit(const char *name) : m_name(name) { }

        ~ScopeExit() {
            cout << "Returning from " << m_name << endl;
        }
    } se(__PRETTY_FUNCTION__);

    try {
        if (0 == mode) {
            throw std::runtime_error("runtime_error");
        } else {
            throw "raw char[]";
        }
    } catch (const std::exception &ex) {
        cerr << "Caught a std::exeption [" << ex.what() << "]" << endl;
    } catch (...) {
        cerr << "Caught an unknown exception" << endl;
    }
}

int main()
{
    cout << "Before foo(0)" << endl;
    foo(0);
    cout << "After foo(0)" << endl;
    cout << "Before foo(1)" << endl;
    foo(1);
    cout << "After foo(1)" << endl;

    cout << endl;
    cout << "Btw __PRETTY_FUNCTION__ == \"" << __PRETTY_FUNCTION__ << "\" has the type of \""
        << typeid(__PRETTY_FUNCTION__).name() << "\"" << endl;

    // can we initialize char[] by __PRETTY_FUNCTION__?
    // const char foo[] = __PRETTY_FUNCTION__; // no!
    // const char foo[1024] = __PRETTY_FUNCTION__; // no!
    // char foo[] = __PRETTY_FUNCTION__; // no!
    // char foo[1024] = __PRETTY_FUNCTION__; // no!
}
