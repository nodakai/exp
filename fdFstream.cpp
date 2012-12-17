#if 0

#define protected public

#include <iostream>
#include <fstream>
#include <tr1/memory>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
    vector<tr1::shared_ptr<ifstream> > ifVec;
    for (int i = 1; i < argc; ++i) {
        tr1::shared_ptr<ifstream> pIf(new ifstream(argv[1]));
        ifVec.push_back(pIf);

        // cout << pIf->fd() << endl;
        cout << pIf->rdbuf()->_M_file.fd() << endl;
    }
}

#else

#include <iostream>
#include <fstream>
#include <tr1/memory>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
    vector<tr1::shared_ptr<ifstream> > ifVec;
    for (int i = 1; i < argc; ++i) {
        tr1::shared_ptr<ifstream> pIf(new ifstream(argv[1]));
        ifVec.push_back(pIf);

        struct GetFd : basic_filebuf<char> {
            int fd() { return _M_file.fd(); }
        };
        cout << static_cast<GetFd *>(pIf->rdbuf())->fd() << endl;
    }
}

#endif
