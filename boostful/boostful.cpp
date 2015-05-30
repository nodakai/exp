#include <iostream>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/asio.hpp>

using namespace std;
namespace b = boost;
namespace po = b::program_options;

static void asioServer() {
}

static void asioClientHbKernel() {
}

class AsioClient {
    AsioClient() {
    }

    ~AsioClient() {
    }
};

static void asioClient(const string &host, const string &port) {
    namespace ip = b::asio::ip;

    b::asio::io_service ioSvc;
    ip::tcp::socket sock(ioSvc);
    ip::tcp::resolver resolver(ioSvc);
    b::asio::connect(sock, resolver.resolve(ip::tcp::resolver::query(host, port)));
}

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("lexical_cast", po::value<string>(), "tries lexical_cast")
        ("asio_server", "TCP server by asio")
        ("asio_client", "TCP server by asio")
        ("sigint", "receive SIGINT by asio")
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

    if (vm.count("sigint")) {
        struct C {
            static void handle(const b::system::error_code &e, int signal) {
                if (e) {
                    cout << "Error " << e << endl;
                } else {
                    cout << "SIGINT was caught." << endl;
                }
            }
        };

        b::asio::io_service ioSvc;
        b::asio::signal_set signals(ioSvc, SIGINT);
        signals.async_wait(&C::handle);
        ioSvc.run();
        cout << ": leaving..." << endl;
        return 0;
    }

    return 0;
}
