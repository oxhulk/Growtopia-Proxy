#pragma once
#include <string>
#include "proton/variant.hpp"
#include "enet/include/enet.h"
#include "world/world.h"

class Server {
   private:
    ENetHost* proxy_server;
    ENetHost* real_server;
    ENetPeer* server_peer;
    ENetPeer* gt_peer;
    void handle_outgoing();
    void handle_incoming();
    bool connect();
    void disconnect(bool reset);
   public:
    int user = 0;
    int token = 0;
    std::string ip;
    std::string meta;
    int port = 17182;
    int proxyport = 17191;
    World world;
    bool start();
    void quit();
    bool setup_client();
    void redirect_server(variantlist_t& varlist);
    void send(bool client, int32_t type, uint8_t* data, int32_t len);
    void send(bool client, variantlist_t& list, int32_t netid = -1, int32_t delay = 0);
    void send(bool client, std::string packet, int32_t type = 2);
    void poll();
};
extern Server* server;
