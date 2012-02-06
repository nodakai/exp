#include <iostream>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
namespace b = boost;
namespace po = b::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("lexical_cast", po::value<string>(), "tries lexical_cast")
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
        cout << b::format("debug level == %1%") % vm["debug"].as<int>() << endl;
    }

    if (vm.count("lexical_cast")) {
        const string before = vm["lexical_cast"].as<string>();
        const int after = b::lexical_cast<int>(before);
        cout << b::format("lexical_cast<int>(%1%) == %2%") % before % after << endl;
    }

    return 0;
}
