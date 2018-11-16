#include <iostream>
#include "server.hpp"

int main() {
    Server s(60000);
    s.run();
    return 0;
}
