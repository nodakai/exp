#include <time.h>
#include <iostream>

using namespace std;

void printTmYear0(const tm &tm0)
{
    cout << tm0.tm_year << " " << tm0.tm_mday << " " << tm0.tm_sec << " " << tm0.tm_yday << endl;
}

void printTmYear(const tm &tm0 = tm())
{
    printTmYear0(tm0);
}

int main()
{
    printTmYear();
    printTmYear0(tm());

    tm tm1;
    printTmYear(tm1);
    printTmYear0(tm1);

    tm tm2 = {};
    printTmYear(tm2);
    printTmYear0(tm2);

    tm tm3 = tm();
    printTmYear(tm3);
    printTmYear0(tm3);
}
