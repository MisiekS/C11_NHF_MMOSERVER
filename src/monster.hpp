#ifndef MONSTER_HPP
#define MONSTER_HPP 1

#include <atomic>
#include "directions.hpp"
#include "actions.hpp"

class Monster {
    static unsigned id_c;
    unsigned id;
    std::atomic_int health;
    short x, y;
    std::atomic<Directions> direction;

public:
    Monster(int hp, short x, short y)
            : health{hp}, x{x}, y{y}, direction{South}, id{id_c++} {}

    short getX() const noexcept { return x; }
    short getY() const noexcept { return y; }
    short getId() const noexcept { return id; }
    int getHealth() const noexcept { return health; }
    Directions getDir()const{return direction;}
    void hit(int value){
        health-=value;
    }
};


#endif