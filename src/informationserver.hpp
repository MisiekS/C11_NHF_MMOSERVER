#ifndef INFORMATIONSERVER_HPP
#define INFORMATIONSERVER_HPP 1

#include <atomic>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <ctime>
#include <forward_list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <algorithm>

#include "database.hpp"
#include "monster.hpp"
#include "tile.hpp"
#include "command.hpp"

using boost::asio::ip::tcp;

// logging, new player, new mob, map, tiles, mob dies
class InformationServer
        : public std::enable_shared_from_this<InformationServer> {
    std::map<unsigned short, std::shared_ptr<Player>> &players;
    std::mutex &players_guard;
    unsigned short port;
    tcp::acceptor acceptor_;
    std::shared_ptr<Database> db;
    std::queue<std::vector<char>> &messages;
    boost::asio::io_context &io_context;
    std::set<std::shared_ptr<Monster>> &monsters;
    std::mutex &monsters_guard;
    Tile &field;

public:
    InformationServer(boost::asio::io_context &io_context,
                      std::map<unsigned short, std::shared_ptr<Player>> &players,
                      std::mutex &players_guard,
                      std::set<std::shared_ptr<Monster>> &monsters,
                      std::mutex &monsters_guard, unsigned short port,
                      std::queue<std::vector<char>> &messages,
                      Tile &field)
            : players{players},
              players_guard{players_guard},
              monsters{monsters},
              monsters_guard{monsters_guard},
              port{port},
              io_context{io_context},
              acceptor_{io_context, tcp::endpoint(tcp::v4(), port)},
              messages{messages},
              field{field},
              db{std::make_shared<Database>("db.sqlite3")} {}

    void run();

    static void MonsterCreation(bool &run, std::set<std::shared_ptr<Monster>> &monsters,
                                std::mutex &monsters_guard,
                                Tile &field,
                                std::vector<std::pair<char,char>>& areas);

private:
    void handleRead(std::shared_ptr<tcp::socket> socket,
                    std::shared_ptr<std::vector<char>> buff,
                    const boost::system::error_code &err,
                    std::shared_ptr<std::vector<char>> cmd);

    void startAccept();

    void loop(bool &run_loop);

    void handleSend(const boost::system::error_code &error,
                    std::size_t bytes_transferred);


};


#endif