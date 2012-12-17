#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

int main()
{
    const int N = 1000 * 1000;
    vector<ofstream *> ofVec;
    ofVec.reserve(N);

    int i;
    for (i = 0; i < N; ++i) {
        ostringstream oss;
        oss << "/tmp/foo/file-" << setfill('0') << setw(6) << i << ".dat" ;
        ofstream *ofs = new ofstream(oss.str().c_str());
        if ( ! *ofs)
            break;
        ofVec.push_back(ofs);
    }
    cout << "i == " << i << endl;
}
