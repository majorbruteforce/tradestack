#include <iostream>

#include "network.hpp"
#include "notifier.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));

    Server            srv(port);
    const std::string TSLA = "TSLA";

    srv.manager.new_instrument(TSLA);

    Notifier::instance().addGroup("F1");

    if (!srv.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    srv.run();
    srv.stop();
    return 0;
}