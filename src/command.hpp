
#ifndef SERVER_COMMAND_H
#define SERVER_COMMAND_H

#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include "actions.hpp"
#include "directions.hpp"

class Command {
    std::vector<char> message;
public:
    Command & add(short value);
    Command & add(unsigned short value);
    Command & add(int value);
    Command & add(const std::string& value);
    Command & add(char value);
    Command & add(const Actions & action);
    Command & add(const Directions & dir);
    std::vector<char> getMessage();
    static Command create();
    static std::vector<std::string> get(std::vector<char> & value);
};


#endif //SERVER_COMMAND_H
