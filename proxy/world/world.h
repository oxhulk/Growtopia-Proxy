#pragma once
#include <string>
#include <vector>
#include "player.h"
#include "packet.h"
#include "utils/utils.h"

class World {
public:
    std::string name{};
    std::vector<Player> players{};
    Player local{};
    bool connected{};
    void load_from_mem(gameupdatepacket_t* packet);
};