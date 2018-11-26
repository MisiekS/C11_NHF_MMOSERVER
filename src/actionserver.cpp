#include <boost/lexical_cast.hpp>
#include "actionserver.hpp"

void ActionServer::run() {
    buffer = std::vector<char>(256);
    start_receive();
    bool run_loop = true;
    std::thread loop_thread(&ActionServer::loop, this, std::ref(run_loop));
    io_context.run();
    run_loop = false;
    loop_thread.join();
}

void ActionServer::start_receive() {
    socket_.async_receive_from(
            boost::asio::buffer(buffer), remote_endpoint_,
            boost::bind(&ActionServer::handle_receive, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}

void ActionServer::handle_receive(const boost::system::error_code &error,
                                  std::size_t /*bytes_transferred*/) {
    if (!error || error == boost::asio::error::message_size) {
        //      std::cout << "udp:" << buffer.size() << " ";
//        std::cout << std::string(buffer.begin(), buffer.end()) << std::endl;

        auto cmd = Command::getUdp(buffer);
        buffer.reserve(256);
        if (cmd.size() > 2) {
            std::shared_lock<std::shared_mutex> player_lock{players_guard};
            auto playerr = players.find(boost::lexical_cast<unsigned short>(cmd[1]));
            auto player = playerr->second;
            if (playerr != players.end() && player->getSecret() == cmd[2]) {
                {
                    std::unique_lock<std::mutex> lock(endpoints_of_last_min_guard);
                    auto e = endpoints_of_last_min.find(std::make_pair(remote_endpoint_, player));
                    if (e == endpoints_of_last_min.end())
                        endpoints_of_last_min.insert(
                                std::make_pair(std::make_pair(remote_endpoint_, player),
                                               std::chrono::steady_clock::now()));
                    else
                        endpoints_of_last_min[std::make_pair(remote_endpoint_,
                                                             player)] = std::chrono::steady_clock::now();
                }
                switch (cmd[0][0]) {
                    case 'A':
                        if (player->getActions() == Actions::Attack)
                            break;
                        if (player->getAction().cancelable())
                            player->setAction(Action{PlayerActionTimes::Attack,
                                                     Actions::Attack});
                        break;
                    case 'M':
                        Directions dir;
                        switch (cmd[3][0]) {
                            case 'N':
                                dir = Directions::North;
                                break;
                            case 'S':
                                dir = Directions::South;
                                break;
                            case 'W':
                                dir = Directions::West;
                                break;
                            case 'E':
                                dir = Directions::East;
                                break;
                            default:
                                dir = Directions::South;
                                break;
                        }

                        if (player->getActions() == Actions::Move && player->getDir() == dir)
                            break;

                        if (player->getAction().cancelable()) {
                            player->setDirection(dir);
                            bool blocking = false;
                            switch (dir) {
                                case West:
                                    blocking = field.blocking(
                                            static_cast<short>(player->getX() - 1), player->getY());
                                    break;
                                case East:
                                    blocking = field.blocking(
                                            static_cast<short>(player->getX() + 1), player->getY());
                                    break;
                                case North:
                                    blocking = field.blocking(player->getX(),
                                                              static_cast<short>(player->getY() -
                                                                                 1));
                                    break;
                                case South:
                                    blocking = field.blocking(player->getX(),
                                                              static_cast<short>(player->getY() +
                                                                                 1));
                                    break;
                            }
                            if (blocking) {
                                if (player->getActions() != Actions::Move_animation)
                                    player->setAction(Action{PlayerActionTimes::Move_animation,
                                                             Actions::Move_animation});
                            }else
                                player->setAction(Action{PlayerActionTimes::Move,
                                                         Actions::Move});

                        }
                        break;
                    case 'S':
                        if(player->getActions()==Actions::Move_animation)
                            player->setAction(Action{PlayerActionTimes::Stand,
                                                     Actions::Stand});
                        else
                            player->getAction().setLast(true);
                        break;
                    default:
                        break;
                }
            }
        }


    }

    start_receive();
}

void ActionServer::loop(bool &run_loop) {
    srand(time(NULL));

    std::cout << "action server loop" << std::endl;
    while (run_loop) {
        std::list<std::pair<std::pair<short, short>, std::vector<char>>> current_messages;
        {
            std::shared_lock<std::shared_mutex> lock(players_guard);
            for (auto & p:players) {
                if (p.second->getActions() == Actions::Attack &&
                    p.second->getActionpercent() > 50 &&
                    !p.second->getAction().getCompleted()) {
                    p.second->getAction().setCompleted(true);
                    bool monster = false;
                    {
                        std::unique_lock<std::shared_mutex> mlock(monsters_guard);
                        for (auto &m:monsters) {
                            std::pair<short, short> cord;
                            switch (p.second->getDir()) {
                                case North:
                                    cord.first = p.second->getX();
                                    cord.second = p.second->getY() - 1;
                                    break;
                                case South:
                                    cord.first = p.second->getX();
                                    cord.second = p.second->getY() + 1;
                                    break;
                                case East:
                                    cord.first = p.second->getX() + 1;
                                    cord.second = p.second->getY();
                                    break;
                                case West:
                                    cord.first = p.second->getX() - 1;
                                    cord.second = p.second->getY();
                                    break;
                            }
                            if ((cord.first == m->getX() && cord.second == m->getY()) ||
                                (cord.first == m->getX() + 1 && cord.second == m->getY()) ||
                                (cord.first == m->getX() + 1 && cord.second == m->getY() + 1) ||
                                (cord.first == m->getX() && cord.second == m->getY() + 1)) {
                                int hit = rand() % 20000 + 10000;

                                current_messages.push_back(
                                        std::make_pair(std::make_pair(cord.first, cord.second),
                                                       Command::create().add('H').add(
                                                               cord.first).add(cord.second).add(
                                                               hit).getMessage()));
                                m->hit(hit);

                                if (m->getHealth() <= 0) {
                                    areas[((m->getX()>>6)+512)|(((m->getY()>>6)+512)<<10)].first--;
                                    messages.push(Command::create().add('K').add(
                                            m->getId()).getMessage());
                                    monsters.erase(std::find(monsters.begin(),monsters.end(),m));
                                }
                                break;
                            }
                        }
                    }
                }

                if ((p.second->getActions() == Actions::Attack ||
                     p.second->getActions() == Actions::Move)
                    && p.second->getActionpercent() >= 99) {
                    if (p.second->getActions() == Actions::Move)
                        switch (p.second->getDir()) {
                            case North:
                                p.second->setY(p.second->getY() - 1);
                                break;
                            case South:
                                p.second->setY(p.second->getY() + 1);
                                break;
                            case East:
                                p.second->setX(p.second->getX() + 1);
                                break;
                            case West:
                                p.second->setX(p.second->getX() - 1);
                                break;
                        }
                    if (p.second->getAction().getLast())
                        p.second->setAction(Action{PlayerActionTimes::Stand, Actions::Stand});
                    else if (p.second->getActions() == Actions::Attack)
                        p.second->setAction(Action{PlayerActionTimes::Attack, Actions::Attack});
                    else {
                        bool blocking = false;
                        switch (p.second->getDir()) {
                            case West:
                                blocking = field.blocking(
                                        static_cast<short>(p.second->getX() - 1), p.second->getY());
                                break;
                            case East:
                                blocking = field.blocking(
                                        static_cast<short>(p.second->getX() + 1), p.second->getY());
                                break;
                            case North:
                                blocking = field.blocking(p.second->getX(),
                                                          static_cast<short>(p.second->getY() -
                                                                             1));
                                break;
                            case South:
                                blocking = field.blocking(p.second->getX(),
                                                          static_cast<short>(p.second->getY() +
                                                                             1));
                                break;
                        }
                        if (blocking)
                            p.second->setAction(Action{PlayerActionTimes::Move_animation,
                                                     Actions::Move_animation});
                        else
                            p.second->setAction(Action{PlayerActionTimes::Move,
                                                     Actions::Move});
                    }
                }
                current_messages.push_back(
                        std::make_pair(std::make_pair(p.second->getX(), p.second->getY()),
                                       Command::create().add('X').add(p.second->getId()).add(
                                               p.second->getX()).add(
                                               p.second->getY()).add(p.second->getActions()).add(
                                               p.second->getActionpercent()).add(
                                               p.second->getDir()).add(
                                               p.second->getActionNumber()).getMessage()));
            }
        }

        {
            std::shared_lock<std::shared_mutex> lock(monsters_guard);
            for (auto const & m:monsters)
                current_messages.push_back(
                        std::make_pair(std::make_pair(m->getX(), m->getY()),
                                       Command::create().add('Y').add(m->getId()).add(
                                               m->getHealth()).add(
                                               m->getDir()).getMessage()));
        }
        {
            std::unique_lock<std::mutex> lock(endpoints_of_last_min_guard);
            for (auto r: endpoints_of_last_min)
                for (auto m: current_messages)
                    if (Tile::indistance(m.first.first, m.first.second, r.first.second->getX(),
                                         r.first.second->getY(), 20))
                        socket_.async_send_to(boost::asio::buffer(m.second), r.first.first,
                                              boost::bind(&ActionServer::handle_send, this,
                                                          boost::asio::placeholders::error,
                                                          boost::asio::placeholders::bytes_transferred));
        }

        auto time = std::chrono::steady_clock::now();
        {
            std::unique_lock<std::mutex> lock(endpoints_of_last_min_guard);
            std::for_each(endpoints_of_last_min.begin(), endpoints_of_last_min.end(),
                          [&](auto &it) {
                              if (std::chrono::duration_cast<std::chrono::microseconds>(
                                      time - it.second).count() > 60000000)
                                  endpoints_of_last_min.erase(it.first);
                          });
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
}


void ActionServer::handle_send(const boost::system::error_code &error,
                               std::size_t /*bytes_transferred*/) {
    if (error)
        std::cout << "udp error" << std::endl;
}
