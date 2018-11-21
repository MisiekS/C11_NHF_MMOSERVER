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
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::milliseconds timer;
    bool completed;
    bool last = false;

public:
    Action(std::chrono::milliseconds timer, Actions action) noexcept: timer{timer}, action{action},
                                                                      completed{false} {
        start = std::chrono::high_resolution_clock::now();
    }

    Action(Action &&) = default;

    Action(const Action &) = default;

    Action &operator=(const Action &) = default;

    Action &operator=(Action &&) = default;

    bool cancelable() {
        //todo make more specific with optimalization
        return timer == std::chrono::milliseconds{0} || getPercent() < 30;
    }

    bool getCompleted() {
        return completed;
    }

    void setCompleted(bool c) {
        completed = c;
    }

    bool getLast() {
        return last;
    }

    void setLast(bool c) {
        last = c;
    }

    Actions getAction() const noexcept {
        return action;
    }

    short getPercent() const noexcept {
        return timer == std::chrono::milliseconds{0} ? static_cast<short>(100) : static_cast<short>(100*(
                std::chrono::duration<long, std::milli>(std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - start))).count()/timer.count());
    }


};

#endif