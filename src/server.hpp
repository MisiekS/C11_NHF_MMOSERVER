#ifndef SERVER_HPP
#define SERVER_HPP 1

#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <fstream>
#include <boost/lexical_cast.hpp>

#include "actionserver.hpp"
#include "informationserver.hpp"
#include "monster.hpp"
#include "tile.hpp"
#include "player.hpp"

class Server {
    std::map<unsigned short, std::shared_ptr<Player>> players;
    std::mutex players_guard;
    Tile field;
    std::set<std::shared_ptr<Monster>> monsters;
    std::mutex monsters_guard;
    std::queue<std::vector<char>> messages;
    unsigned short port;

public:
    Server(unsigned short port) : port{port}, field{monsters, players} {

    }

    void run();
};

#endif