#ifndef PLAYER_HPP
#define PLAYER_HPP 1

#include <atomic>
#include <string>

#include <boost/asio.hpp>
#include "actions.hpp"
#include "directions.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Player {
    const std::string username;
    const unsigned short id;
    short x, y;
    Directions direction;
    Action action;
    int action_counter = 0;
    std::shared_ptr<tcp::socket> tcp_socket;
    std::string secret;

public:
    Player(std::string &username, unsigned short id, short x, short y, std::string &secret)
            : username{username}, id{id}, x{x}, y{y}, direction{South}, secret{secret},
              action{std::chrono::milliseconds{0}, Actions::Stand} {}

    void setAction(Action &&a) {
        action = a;
        ++action_counter;
    }

    void setX(short x_) { x = x_; }

    void setY(short y_) { y = y_; }

    void set_tcp_socket(std::shared_ptr<tcp::socket> &t) { tcp_socket = t; }

    void setDirection(const Directions &dir) {
        direction = dir;
    }

    auto get_tcp_socket() { return tcp_socket; }

    unsigned short getId() const { return id; }

    const std::string &getUsername() const { return username; }

    short getX() const { return x; }

    short getY() const { return y; }

    const std::string &getSecret() const { return secret; }

    Actions getActions() const { return action.getAction(); }

    Action &getAction() { return action; }

    short getActionpercent() const { return action.getPercent(); }

    Directions getDir() const { return direction; }

    int getActionNumber() const noexcept {
        return action_counter;
    }
};

#endif
