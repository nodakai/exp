typedef void *Ptr;
typedef void const *CPtr;
// #define Ptr void*

class C {
    int x;
public:
    C() : x(0) { }
    CPtr p() const { return &x; }
    Ptr p() { return &x; }
};

int main()
{
    C c;
    c.p();
    const C cc;
    cc.p();
    const char * * const * const * * * const guessWhatIAmPtr = 0;
}
