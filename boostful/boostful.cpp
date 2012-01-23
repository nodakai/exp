#include <iostream>

#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("debug", po::value<int>(), "set debug level")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cerr << desc << endl;
        return 10;
    }

    if (vm.count("debug")) {
        cout << "debug level == " << vm["debug"].as<int>() << endl;
    }

    return 0;
}
