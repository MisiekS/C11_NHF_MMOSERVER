#include "command.hpp"

Command & Command::add(short value){
    auto str = std::to_string(static_cast<int>(value));
    message.insert(message.end(),str.begin(),str.end());
    message.push_back(' ');
    return *this;
}

Command & Command::add(unsigned short value){
    auto str = std::to_string(static_cast<unsigned>(value));
    message.insert(message.end(),str.begin(),str.end());
    message.push_back(' ');
    return *this;
}

Command & Command::add(int value){
    auto str = std::to_string(value);
    message.insert(message.end(),str.begin(),str.end());
    message.push_back(' ');
    return *this;
}


Command & Command::add(const std::string& value){
    message.insert(message.end(),value.begin(),value.end());
    message.push_back(' ');
    return *this;
}

Command & Command::add(char value){
    message.push_back(value);
    message.push_back(' ');
    return *this;
}

Command & Command::add(const Actions & action ){
    switch (action){
        case Attack:
            add("attack");
            break;
        case Move:
            add("move");
            break;
        case Move_animation:
            add("movea");
            break;
        case Stand:
            add("stand");
            break;
    }
    return *this;
}

Command & Command::add(const Directions & dir ){
    switch (dir){
        case North:
            add("north");
            break;
        case West:
            add("west");
            break;
        case South:
            add("south");
            break;
        case East:
            add("east");
            break;
    }
    return *this;
}

std::vector<char> Command::getMessage(){
    message.push_back('\n');
    return message;
}

Command Command::create() {
    return Command();
}

std::vector<std::string> Command::get(std::vector<char> & value){
    auto check = std::find(value.begin(),value.end(),'\n');
    if(check!=value.end()) {
        std::stringstream ss(std::string(value.begin(),check));
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        value.erase(value.begin(),check+1);
        return {begin, end};
    }
    else return {};
}

std::vector<std::string> Command::getUdp(std::vector<char> & value){
    auto check = std::find(value.begin(),value.end(),'\n');
    if(check!=value.end()) {
        std::stringstream ss(std::string(value.begin(),check));
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        return {begin, end};
    }
    else return {};
}

