#include "server.hpp"

void Server::run() {
    bool doexit = false;
//todo thread
    bool monster_thread_run = true;
    std::thread ms{InformationServer::MonsterCreation, std::ref(monster_thread_run),
                   std::ref(monsters), std::ref(monsters_guard),
                   std::ref(players),std::ref(players_guard), std::ref(field)};

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
        std::getline(std::cin, cmd);
        if (cmd == "exit") doexit = true;
        if (cmd[0] == 's') {
            cmd += '\n';
            std::vector<char> cc{cmd.begin(), cmd.end()};
            auto c = Command::get(cc);
            {
                std::lock_guard<std::mutex> lock{monsters_guard};
                monsters.insert(std::make_shared<Monster>(boost::lexical_cast<int>(c[1]),
                                                          boost::lexical_cast<short>(c[2]),
                                                          boost::lexical_cast<short>(c[3])));
            }
        }
    }
    monster_thread_run = false;
    ms.join();
    io_as.stop();
    as_t.join();
    io_is.stop();
    is_t.join();
}