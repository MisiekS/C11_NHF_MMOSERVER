#include "informationserver.hpp"

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
        if (monsters.size() < 10000 && (rand() % 100) == 1) {
            short x, y;
            do {
                x = static_cast<short>(rand());
                y = static_cast<short>(rand());
            } while (field.blocking(x, y) ||
                     field.blocking(x + 1, y) ||
                     field.blocking(x + 1, y + 1) ||
                     field.blocking(x, y + 1));
            std::lock_guard<std::mutex> lock(monsters_guard);
            auto newmonster = std::make_shared<Monster>(rand() % 1000000 + 10000, x, y);
            monsters.insert(newmonster);
            messages.push(Command::create().add('M').add(newmonster->getId()).add(
                    newmonster->getHealth()).add(
                    newmonster->getX()).add(
                    newmonster->getY()).getMessage());
        }
        if (count % 100)
            messages.push(Command::create().add('p').add("pingellekgeco").getMessage());//debug
        while (!messages.empty()) {
            std::lock_guard<std::mutex> lock(players_guard);
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
            std::cout << std::string(f.begin(), f.end()) << std::endl;
            cmd->insert(cmd->end(), f.begin(), std::find(f.begin(), f.end(), 0));
            std::cout << std::string(cmd->begin(), cmd->end()) << std::endl;
        }

        auto received = Command::get(*cmd);

        if (!received.empty()) {
            switch (received[0][0]) {
                case '0':
                    if (received.size() > 2) {
                        // login

                        //todo make login smarter
                        //i mean it can handle logins that already logged in

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
                                std::lock_guard<std::mutex> lock(players_guard);
                                if (players.find(player->getId()) != players.end())
                                    players.erase(player->getId());
                                players.insert(std::make_pair(player->getId(), player));
                            }

                            auto loginmessage = Command::create().add('0').add(username)
                                    .add(player->getId()).add(player->getX())
                                    .add(player->getY()).getMessage();

                            messages.push(loginmessage);
                            //todo not all please
                            std::queue<std::vector<char>> login_messages;
                            {
                                std::lock_guard<std::mutex> lock(players_guard);
                                for (auto p:players)
                                    if (Tile::indistance(p.second->getX(), p.second->getY(),
                                                       player->getX(), player->getY(),50))
                                        login_messages.push(Command::create().add('0').add(
                                                        p.second->getUsername())
                                                                    .add(p.second->getId()).add(
                                                        p.second->getX())
                                                                    .add(p.second->getY()).getMessage());
                            }
                            {
                                std::lock_guard<std::mutex> lock(monsters_guard);
                                for (auto m:monsters)
                                    if (Tile::indistance(m->getX(),m->getY(),player->getX(),player->getY(),50))
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
                case '1':
                    if (received.size() > 1) {
                        // logout
                        // such an unsafe method
                        auto player_id = boost::lexical_cast<unsigned short>(received[1]);
                        auto player_q = std::find_if(players.begin(), players.end(), [&](auto &it) {
                            return it.second->get_tcp_socket() == socket;
                        });
                        if (player_q == players.end())break;
                        auto player = player_q->second;
                        if (player->getId() != player_id)break;
                        db->save(player);
                        players.erase(players.find(player_id));
                        std::string username = player->getUsername();
                        auto logoutmessage = Command::create().add('1').add(
                                player->getId()).getMessage();
                        messages.push(logoutmessage);
                        socket->close();
                        std::cout << player_id << " succesfully logged out" << std::endl;
                        return;
                    }
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
                            std::string succ_message = "nope " + username;
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

                    //todo asking data by id for monster/ for player
                case 'M':
                    break;
                case 'P':
                    break;

                default:
                    break;
            }
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
    //TODO
    if (error) {
        std::cout << "tcp error" << std::endl;
        //too much error -> logout
    }
}
