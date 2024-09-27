#pragma once
#include <string>

namespace gt {
    extern bool connecting;
    extern bool in_game;
    void send_log(std::string text);
}
