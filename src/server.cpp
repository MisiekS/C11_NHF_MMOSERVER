#include "server.hpp"

void Server::run() {
    bool doexit = false;

    //todo fill areas please
    std::chrono::high_resolution_clock::time_point start =std::chrono::high_resolution_clock::now();

    for (short i = 0; i < 1024; ++i)
        for (short j = 0; j < 1024; ++j) {
            int nb = 0;
            for (short x = 0; x < 64; ++x)
                for (short y = 0; y < 64; ++y)
                    if (field.blocking(i * 64 + x, j * 64 + y))
                        nb++;
            areas[i + j * 1024].first = 0;
            areas[i + j * 1024].second = static_cast<char>((1024-nb)/10);
        }

    std::chrono::high_resolution_clock::time_point end =std::chrono::high_resolution_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::chrono::duration<long, std::milli> int_usec = int_ms;
    std::cout << int_usec.count() <<std::endl;

    bool monster_thread_run = true;
    std::thread ms{InformationServer::MonsterCreation, std::ref(monster_thread_run),
                   std::ref(monsters), std::ref(monsters_guard), std::ref(field), std::ref(areas)};

    boost::asio::io_context io_is;
    boost::asio::io_context io_as;
    InformationServer is{io_is, players, players_guard, monsters,
                         monsters_guard, port, messages, field};


    ActionServer as{io_as, players, players_guard, monsters,
                    monsters_guard, port, messages, field, areas};

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
    std::cout << "server shutting down"<<std::endl;
    monster_thread_run = false;
    io_as.stop();
    as_t.join();
    io_is.stop();
    is_t.join();
    ms.join();
}