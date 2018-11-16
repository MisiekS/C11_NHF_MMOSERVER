#include "server.hpp"

void Server::run() {
    bool doexit = false;

    // creating monsters here please
    while (monsters.size() < 5000) {
        short x, y;
        do {
            x = static_cast<short>(rand());
            y = static_cast<short>(rand());
        } while (field.blocking(x, y) ||
                 field.blocking(x + 1, y) ||
                 field.blocking(x + 1, y + 1) ||
                 field.blocking(x, y + 1));
        std::lock_guard<std::mutex> lock(monsters_guard);
        monsters.insert(std::make_shared<Monster>(rand() % 1000000 + 10000, x, y));
    }


    boost::asio::io_context io_is;
    boost::asio::io_context io_as;
    InformationServer is{io_is, players, players_guard, monsters,
                         monsters_guard, port, messages, field};


    ActionServer as{io_as, players, players_guard, monsters,
                    monsters_guard, port, messages, field};

    std::thread as_t{&ActionServer::run, &as};
    std::thread is_t(&InformationServer::run, &is);

    while (!doexit) {
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "exit") doexit = true;
    }
    io_as.stop();
    as_t.join();
    io_is.stop();
    is_t.join();
}