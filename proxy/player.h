#pragma once
#include <string>
#include "proton/vector.hpp"

class Player {
   public:
    std::string name;
    std::string country;
    int netid;
    int userid;
    vector2_t pos;
    bool invis;
    bool mod;

    Player() {
    
    }
    bool operator==(const Player& right) {
        return netid == right.netid && userid == right.userid;
    }
    Player(std::string name, int netid, int uid) {
        this->name = name;
        this->netid = netid;
        this->userid = uid;
    }
};