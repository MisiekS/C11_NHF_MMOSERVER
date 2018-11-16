#ifndef ACTIONSERVER_HPP
#define ACTIONSERVER_HPP 1

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <chrono>
#include <set>
#include <list>
#include <iostream>
#include <string>
#include <ctime>
#include "monster.hpp"
#include "player.hpp"
#include "command.hpp"
#include "tile.hpp"
#include "actions.hpp"

using boost::asio::ip::udp;

// moving, attacking, hp indicator
class ActionServer {
    std::map<unsigned short, std::shared_ptr<Player>> &players;
    std::mutex &players_guard;
    unsigned short port;
    Tile &field;
    std::queue<std::vector<char>> &messages;
    boost::asio::io_context &io_context;
    std::set<std::shared_ptr<Monster>> &monsters;
    std::mutex &monsters_guard;
    std::map<std::pair<udp::endpoint,std::shared_ptr<Player>>, std::chrono::steady_clock::time_point> endpoints_of_last_min;
    std::mutex endpoints_of_last_min_guard;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::vector<char> buffer;
    //hits

public:
    ActionServer(boost::asio::io_context &io_context,
                 std::map<unsigned short, std::shared_ptr<Player>> &players,
                 std::mutex &players_guard,
                 std::set<std::shared_ptr<Monster>> &monsters,
                 std::mutex &monsters_guard, unsigned short port,
                 std::queue<std::vector<char>> &messages, Tile &field
    )
            : players{players},
              players_guard{players_guard},
              monsters{monsters},
              monsters_guard{monsters_guard},
              port{port},
              io_context{io_context},
              messages{messages},
              field{field},
              socket_{io_context, udp::endpoint(udp::v4(), port)} {}

    void run();

private:
    void start_receive();

    void handle_receive(const boost::system::error_code &error,
                        std::size_t /*bytes_transferred*/);

    void handle_send(const boost::system::error_code &error,
                        std::size_t /*bytes_transferred*/);

    void loop(bool &run_loop);
};

#endif