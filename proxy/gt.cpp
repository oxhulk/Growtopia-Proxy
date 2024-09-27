#include "gt.hpp"
#include "gt.hpp"
#include "packet.h"
#include "server.h"


bool gt::connecting = false;
bool gt::in_game = false;
void gt::send_log(std::string text) {
    server->send(true, "action|log\nmsg|" + text, NET_MESSAGE_GAME_MESSAGE);
}