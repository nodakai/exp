#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

#include <cstdlib>

#include <ncurses.h>
#include <getopt.h>

/*  Misc Utilities  */

static long myAtoi(const char *str)
{
    char *end;
    const long ret = ::strtol(str, &end, 10);
    if ('\0' != *end) {
        std::ostringstream oss;
        oss << "Invalid integer literal: [" << str << "]";
        const string &s = oss.str();
        cerr << s << endl;
        throw std::runtime_error(s);
    }
    return ret;
}

static void printUsage()
{
    cout << "ncurses [-t testCase]" << endl;
    cout << "    -t testCase (int): select pre-defined test cases from http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/" << endl;
    ::exit(10);
}

static void test01()
{
    struct ScopeExit {
        ~ScopeExit() {
            ::endwin();
        }
    } se;

    cout << __PRETTY_FUNCTION__ << endl;
    ::initscr();
    ::printw("Hello world!!\n");
    ::printw("  Hit Enter to exit...");
    ::refresh();
    ::getch();
}

static void test02()
{
    struct ScopeExit {
        ~ScopeExit() {
            ::endwin();
        }
    } se;

    cout << __PRETTY_FUNCTION__ << endl;

    ::initscr();
    ::raw();
    ::keypad(::stdscr, true);
    ::noecho();

    ::printw("Type any character to see it in bold\n");
    int ch = getch();
    if (KEY_F(1) == ch) {
        printw("F1 pressed\n");
    } else {
        ::printw("The pressed key is: ");
        struct ScopeExit {
            ScopeExit() {
                ::attron(A_BOLD);
            }
            ~ScopeExit() {
                ::attroff(A_BOLD);
            }
        } se;

        ::printw("%c", ch);
    }

    ::printw("\n\nHit Enter to exit...");
    ::refresh();
    ::getch();
}

static void test03()
{
    struct ScopeExit {
        ~ScopeExit() {
            ::endwin();
        }
    } se;

    cout << __PRETTY_FUNCTION__ << endl;

    const string mesg("Theory and practice go hand in hand");

    ::initscr();
    int row, col;
    getmaxyx(::stdscr, row, col);
    ::mvprintw(row / 2, (col - mesg.size()) / 2, "%s", mesg.c_str());
    ::mvprintw(row - 3, 0, "This screen has %d rows and %d columns\n", row, col);

    ::printw("\nHit Enter to exit...");
    ::refresh();
    ::getch();
}


/*  Main  */

int main(int argc, char *argv[])
{
    int testCase = -1;

    char optchr;
    while (-1 != (optchr = ::getopt(argc, argv, "v:t:"))) {
        switch (optchr) {
            case 't': testCase = myAtoi(::optarg); break;

            default: printUsage();
        }
    }

    switch (testCase) {
        case  1: test01(); break;
        case  2: test02(); break;
        case  3: test03(); break;
        default: /* pass-through */ break;
    }

    cout << "Quitting..." << endl;
}
