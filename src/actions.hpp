#ifndef ACTIONS_HPP
#define ACTIONS_HPP 1

#include <chrono>

enum Actions {
    Stand,           // just standing
    Move,            // moving in that direction
    Move_animation,  // moving in one place because blocked
    Attack           // attacking in that direction
};

namespace PlayerActionTimes {
    constexpr static std::chrono::milliseconds Attack{1000};
    constexpr static std::chrono::milliseconds Move{1000};
    constexpr static std::chrono::milliseconds Stand{0};
    constexpr static std::chrono::milliseconds Move_animation{0};

};

class Action {
    Actions action;
    std::chrono::steady_clock::time_point start;
    std::chrono::milliseconds time;
    bool completed;

public:
    Action(std::chrono::milliseconds time, Actions action) noexcept: time{time}, action{action}, completed{false} {
        start = std::chrono::steady_clock::now();
    }

    Action(Action &&) = default;

    Action(const Action &) = default;

    Action &operator=(const Action &) = default;

    Action &operator=(Action &&) = default;

    bool cancelable() {
        //todo make more specific with optimalization
        return time == std::chrono::milliseconds{0} ||
               std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start) <
               time * 0.3;
    }

    bool getCompleted(){
        return completed;
    }

    void setCompleted(bool c){
        completed=c;
    }

    Actions getAction() const noexcept {
        return action;
    }

    short getPercent() const noexcept {
        return time == std::chrono::milliseconds{0} ? static_cast<short>(100) : static_cast<short>(
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start) /
                time);
    }


};

#endif