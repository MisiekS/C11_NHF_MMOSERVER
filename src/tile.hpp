#ifndef TILE_HPP
#define TILE_HPP 1

#include <utility>
#include <memory>
#include <set>
#include <map>
#include <cmath>
#include "player.hpp"
#include "monster.hpp"

class Tile {
    std::set<std::shared_ptr<Monster>> &monsters;
    std::map<unsigned short, std::shared_ptr<Player>> players;

public:
    Tile(std::set<std::shared_ptr<Monster>> &monsters, std::map<unsigned short, std::shared_ptr<Player>> &players)
            : monsters{monsters}, players{players} {}

    bool blocking(short x, short y) const noexcept {
        if (x * x + y * y > 30000 * 30000)return true;

        if (!(x * y % 47))return true;
        for (auto m: monsters)
            if (m->getX() == x || m->getY() == y || m->getX() + 1 == x || m->getY() + 1 == y)
                return true;

        for (auto p:players)
            if (p.second->getX() == x || p.second->getY() == y)
                return true;
            else if (p.second->getActions() == Actions::Move && !p.second->getAction().getCompleted())
                switch (p.second->getDir()) {
                    case South:
                        if(p.second->getX()-1==x && p.second->getY()==y)
                            return true;
                        break;
                    case North:
                        if(p.second->getX()+1==x && p.second->getY()==y)
                            return true;
                        break;
                    case East:
                        if(p.second->getX()==x && p.second->getY()+1==y)
                            return true;
                        break;
                    case West:
                        if(p.second->getX()==x && p.second->getY()-1==y)
                            return true;
                        break;
                }

        return false;
    }
    static bool indistance(short x1,short y1,short x2,short y2,short d){
        return std::abs(x1-x2)<=d && std::abs(y1-y2)<=d;
    }
};

#endif