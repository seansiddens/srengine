#include <iostream>

#include "engine.h"

int main() {
    sren::Engine engine;
    if (!engine.init()) {
        std::cerr << "Failed to init engine!\n";
        return -1;
    }


    engine.shutdown();
}
