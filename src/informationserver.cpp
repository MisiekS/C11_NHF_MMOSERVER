#include "informationserver.hpp"

std::condition_variable players_cv;

void InformationServer::run() {
    startAccept();
    bool run_loop = true;
    std::thread loop_thread(&InformationServer::loop, this, std::ref(run_loop));
    io_context.run();
    run_loop = false;
    loop_thread.join();
}

void InformationServer::loop(bool &run_loop) {
    std::cout << "information server loop" << std::endl;
    srand(time(NULL));
    int count = 0;
    while (run_loop) {
        if (count % 100)
            messages.push(Command::create().add('p').add("pingellekgeco").getMessage());//debug
        while (!messages.empty()) {
            std::shared_lock<std::shared_mutex> lock(players_guard);
            for (auto player : players)
                player.second->get_tcp_socket()->async_send(
                        boost::asio::buffer(messages.front()),
                        boost::bind(&InformationServer::handleSend, this,
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::bytes_transferred));
            if (messages.front()[0] != 'p')
                std::cout << "tcp: "
                          << std::string(messages.front().begin(), messages.front().end());
            messages.pop();
        }
        count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

void InformationServer::startAccept() {
    auto peer = std::make_shared<tcp::socket>(io_context);
    acceptor_.async_accept(
            *peer,
            boost::bind<void>(
                    [this](std::shared_ptr<tcp::socket> socket,
                           const boost::system::error_code &err) {
                        if (!err) {
                            std::cout << socket->remote_endpoint().address().to_string()
                                      << std::endl;  // debug

                            std::shared_ptr<std::vector<char>> buff =
                                    std::make_shared<std::vector<char>>(256, 0);
                            std::shared_ptr<std::vector<char>> cmd =
                                    std::make_shared<std::vector<char>>();
                            socket->async_read_some(
                                    boost::asio::buffer(*buff),
                                    boost::bind(&InformationServer::handleRead, this,
                                                socket,
                                                buff, boost::asio::placeholders::error,
                                                cmd));
                        }
                        startAccept();
                    },
                    peer, boost::asio::placeholders::error));
}

void InformationServer::handleRead(std::shared_ptr<tcp::socket> socket,
                                   std::shared_ptr<std::vector<char>> buff,
                                   const boost::system::error_code &err,
                                   std::shared_ptr<std::vector<char>> cmd) {
    if (!err) {
        auto &f = *buff;
        if (!f.empty()) {
            cmd->insert(cmd->end(), f.begin(), std::find(f.begin(), f.end(), 0));
        }

        auto received = Command::get(*cmd);

        while (!received.empty()) {
            switch (received[0][0]) {
                case '0':
                    if (received.size() > 2) {
                        std::string username = received[1];
                        std::string password = received[2];
                        auto player = db->login(username, password);
                        std::cout << "logging in" << std::endl;
                        if (player != nullptr) {
                            // send back it was success
                            auto suc_message = Command::create().add("yeay").add(username).add(
                                    "withid").add(
                                    player->getId()).add(player->getSecret()).getMessage();
                            socket->async_send(
                                    boost::asio::buffer(suc_message),
                                    boost::bind(&InformationServer::handleSend, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
                            std::cout << "he" << std::endl;
                            player->set_tcp_socket(socket);
                            {
                                std::unique_lock<std::shared_mutex> lock(players_guard);
                                if (players.find(player->getId()) != players.end())
                                    players.erase(player->getId());
                                players.insert(std::make_pair(player->getId(), player));
                            }

                            std::queue<std::vector<char>> login_messages;
                            {
                                std::shared_lock<std::shared_mutex> lock(players_guard);
                                for (auto p:players)
                                    if (Tile::indistance(p.second->getX(), p.second->getY(),
                                                         player->getX(), player->getY(), 20))
                                        login_messages.push(Command::create().add('0').add(
                                                        p.second->getUsername())
                                                                    .add(p.second->getId()).add(
                                                        p.second->getX())
                                                                    .add(p.second->getY()).getMessage());
                            }
                            {
                                std::shared_lock<std::shared_mutex> lock(monsters_guard);
                                for (auto &m:monsters)
                                    if (Tile::indistance(m->getX(), m->getY(), player->getX(),
                                                         player->getY(), 20))
                                        login_messages.push(
                                                Command::create().add('M').add(m->getId()).add(
                                                        m->getHealth()).add(
                                                        m->getX()).add(
                                                        m->getY()).getMessage());
                            }
                            while (!login_messages.empty()) {
                                socket->async_send(
                                        boost::asio::buffer(login_messages.front()),
                                        boost::bind(&InformationServer::handleSend, this,
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));
                                login_messages.pop();
                            }

                            std::cout << player->getId() << " succesfully logged in" << std::endl;
                        } else {
                            // send back it was failed
                            auto suc_message = Command::create().add("nope").getMessage();
                            socket->async_send(
                                    boost::asio::buffer(suc_message),
                                    boost::bind(&InformationServer::handleSend, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
                        }
                    }
                    break;
                case '1':{
                    if (received.size() > 1) {
                        auto player_id = boost::lexical_cast<unsigned short>(received[1]);
                        auto player_q = std::find_if(players.begin(), players.end(), [&](auto &it) {
                            return it.second->get_tcp_socket() == socket;
                        });
                        if (player_q == players.end())break;
                        auto player = player_q->second;
                        if (player->getId() != player_id)break;
                        db->save(player);
                        {
                            std::unique_lock<std::shared_mutex> lock{players_guard};
                            players.erase(player_id);
                        }
                        std::string username = player->getUsername();
                        auto logoutmessage = Command::create().add('1').add(
                                player->getId()).getMessage();
                        messages.push(logoutmessage);
                        socket->async_send(
                                boost::asio::buffer(Command::create().add('1').getMessage()),
                                boost::bind(&InformationServer::handleSend, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
                        socket->close();
                        std::cout << player_id << " succesfully logged out" << std::endl;
                        return;
                    }}
                    break;
                case '2':
                    // registration
                    if (received.size() > 2) {
                        std::string username = received[1];
                        std::string password = received[2];
                        if (db->registration(username, password)) {
                            // send back it success
                            auto suc_reg_message = Command::create().add("yeay").add(
                                    username).getMessage();
                            socket->async_send(
                                    boost::asio::buffer(suc_reg_message),
                                    boost::bind(&InformationServer::handleSend, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
                        } else {
                            auto fail_reg_message = Command::create().add("nope").add(
                                    username).getMessage();
                            socket->async_send(
                                    boost::asio::buffer(fail_reg_message),
                                    boost::bind(&InformationServer::handleSend, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
                        }
                    }
                    break;

                case 'M':
                    if (received.size() > 1) {
                        {
                            std::shared_lock<std::shared_mutex> lock(monsters_guard);
                            auto mmm = std::find_if(monsters.begin(), monsters.end(),
                                                    [&](auto &it) {
                                                        return it->getId() ==
                                                               boost::lexical_cast<int>(
                                                                       received[1]);
                                                    });
                            if (mmm != monsters.end()) {
                                auto mm = *mmm;
                                socket->async_send(
                                        boost::asio::buffer(
                                                Command::create().add('M').add(mm->getId()).add(
                                                        mm->getHealth()).add(
                                                        mm->getX()).add(
                                                        mm->getY()).getMessage()),
                                        boost::bind(&InformationServer::handleSend, this,
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));
                            }
                        }
                    }
                    break;
                case 'P':
                    if (received.size() > 1) {
                        {
                            std::shared_lock<std::shared_mutex> lock(players_guard);
                            auto mm = players[boost::lexical_cast<unsigned short>(received[1])];
                            if (mm != nullptr)
                                socket->async_send(
                                        boost::asio::buffer(Command::create().add('0').add(
                                                        mm->getUsername())
                                                                    .add(mm->getId()).add(
                                                        mm->getX())
                                                                    .add(mm->getY()).getMessage()),
                                        boost::bind(&InformationServer::handleSend, this,
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred));
                        }
                    }
                    break;

                default:
                    break;
            }
            received = Command::get(*cmd);
        }
    }
    buff = std::make_shared<std::vector<char>>(256, 0);
    socket->async_read_some(
            boost::asio::buffer(*buff),
            boost::bind(&InformationServer::handleRead, this, socket, buff,
                        boost::asio::placeholders::error, cmd));
}

void InformationServer::handleSend(const boost::system::error_code &error,
                                   std::size_t bytes_transferred) {
    if (error) {
        std::cout << "tcp error" << std::endl;
    }
}


void InformationServer::MonsterCreation(bool &run, std::vector<std::shared_ptr<Monster>> &monsters,
                                        std::shared_mutex &monsters_guard,
                                        Tile &field,
                                        std::map<unsigned short, std::shared_ptr<Player>> &players,
                                        std::shared_mutex &players_guard,
                                        std::vector<std::pair<char, char>> &areas) {
    srand(time(NULL));
    std::cout << "mobs are spawning" << std::endl;
    while (run) {
        std::set<std::pair<short, short>> ca;
        {
            std::shared_lock<std::shared_mutex> player_lock{players_guard};
            for (auto const &p:players) {
                auto player = p.second;
                ca.insert({
                                  {(player->getX() >> 6) - 1, (player->getY() >> 6) - 1},
                                  {(player->getX() >> 6),     (player->getY() >> 6) - 1},
                                  {(player->getX() >> 6) + 1, (player->getY() >> 6) - 1},
                                  {(player->getX() >> 6) - 1, (player->getY() >> 6)},
                                  {(player->getX() >> 6),     (player->getY() >> 6)},
                                  {(player->getX() >> 6) + 1, (player->getY() >> 6)},
                                  {(player->getX() >> 6) - 1, (player->getY() >> 6) + 1},
                                  {(player->getX() >> 6),     (player->getY() >> 6) + 1},
                                  {(player->getX() >> 6) + 1, (player->getY() >> 6) + 1},
                          });
            }
            std::unique_lock<std::shared_mutex> lock{monsters_guard};
            for (auto &cca: ca) {
                if (areas[(cca.first + 512) | ((cca.second + 512) << 10)].first <
                    areas[(cca.first + 512) | ((cca.second + 512) << 10)].second) {
                    short x = static_cast<short>(rand() % 64) | static_cast<short>(cca.first << 6);
                    short y = static_cast<short>(rand() % 64) | static_cast<short>(cca.second << 6);
                    if ((field.blocking(x, y) || field.blocking(x + 1, y) ||
                         field.blocking(x + 1, y + 1) || field.blocking(x, y + 1)))
                        continue;
                    monsters.push_back(std::make_shared<Monster>(rand() % 500000 + 50000, x, y));
                    ++areas[(cca.first + 512) | ((cca.second + 512) << 10)].first;
                }
            }
        }
        {
            std::unique_lock<std::shared_mutex> lock{monsters_guard};
            for (auto & monster:monsters) {
                bool gotcha = false;
                for (auto const &cca:ca) {
                    if ((monster->getX() >> 6) == cca.first &&
                        (monster->getY() >> 6) == cca.second) {
                        gotcha = true;
                        continue;
                    }
                }
                if (gotcha)continue;
                else {
                    monsters.erase(std::find(monsters.begin(), monsters.end(), monster));
                    break;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}
