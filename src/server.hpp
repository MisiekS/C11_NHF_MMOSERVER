#ifndef SERVER_HPP
#define SERVER_HPP 1
#include <shared_mutex>

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
    std::shared_mutex players_guard;
    Tile field;
    std::vector<std::shared_ptr<Monster>> monsters;
    std::shared_mutex monsters_guard;
    std::queue<std::vector<char>> messages;
    std::vector<std::pair<char,char>> areas{1024*1024};
    unsigned short port;

public:
    explicit Server(unsigned short port) : port{port}, field{monsters, players} {}

    void run();
};

#endif